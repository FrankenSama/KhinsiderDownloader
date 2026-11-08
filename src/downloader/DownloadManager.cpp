#include "DownloadManager.h"
#include <QEventLoop>
#include <QSet>
#include <QTimer>
#include <algorithm>
#include <iterator>

AsyncDownloadManager::AsyncDownloadManager(QObject *parent)
	: QObject(parent), m_limitPerThread(0), m_threadCount(1), m_currentWorkerIndex(0) {
	addWorkers();
}

AsyncDownloadManager::~AsyncDownloadManager() {
	cleanupWorkers();
}

void AsyncDownloadManager::setDownloadLimitPerThread(int limit) {
	// 0 means "no per-worker limit".
	limit = qMax(0, limit);

	QMutexLocker locker(&m_workersMutex);
	if (limit == m_limitPerThread) {
		return;
	}

	m_limitPerThread = limit;
	for (auto &worker: m_workers) {
		worker->setMaxConcurrentDownloads(limit);
	}
}

void AsyncDownloadManager::setThreadCount(int count) {
	count = qMax(1, count);

	if (count == m_threadCount) {
		return;
	}

	if (count > m_threadCount) {
		m_threadCount = count;
		addWorkers();
	} else {
		m_threadCount = count;
		reduceWorkers();
	}
}

void AsyncDownloadManager::reduceWorkers() {
	QMutexLocker locker(&m_workersMutex);
	int workersToRemove = m_workers.size() - m_threadCount;
	if (workersToRemove <= 0) return;

	std::sort(m_workers.begin(), m_workers.end(),
	          [](DownloadWorker *a, DownloadWorker *b) {
		          return a->activeDownloadCount() < b->activeDownloadCount();
	          });

	int markedForRemoval = 0;
	for (DownloadWorker *worker: m_workers) {
		if (markedForRemoval >= workersToRemove) {
			break;
		}
		if (!worker || worker->IsGracefulStopping()) {
			continue;
		}
		QMetaObject::invokeMethod(worker, "prepareForShutdown", Qt::QueuedConnection);
		++markedForRemoval;
	}
}

void AsyncDownloadManager::addWorkers() {
	QMutexLocker locker(&m_workersMutex);
	if (m_shuttingDown) {
		return;
	}

	int nNeededWorkers = m_threadCount - m_threads.size();
	if (nNeededWorkers <= 0) {
		return;
	}

	for (int i = 0; i < nNeededWorkers; ++i) {
		auto thread = new QThread();
		auto worker = new DownloadWorker(m_nextWorkerId++, m_limitPerThread, nullptr);

		worker->moveToThread(thread);

		connect(thread, &QThread::started, worker, &DownloadWorker::start);
		connect(thread, &QThread::finished, worker, &QObject::deleteLater);
		connect(worker, &DownloadWorker::stopped, this, [this, worker, thread]() {
			{
				QMutexLocker locker(&m_workersMutex);
				m_workers.removeOne(worker);
				m_threads.removeOne(thread);
			}
			thread->quit();

			// The user may have raised the thread count while this worker was
			// shutting down. Re-evaluate the pool once the removal is done.
			QTimer::singleShot(0, this, [this]() { addWorkers(); });
		});

		m_threads.append(thread);
		m_allThreads.append(thread);
		m_workers.append(worker);

		thread->start();
	}
}

DownloadWorker *AsyncDownloadManager::getLeastBusyWorker() {
	std::vector<DownloadWorker *> activeWorkers;
	std::copy_if(m_workers.begin(), m_workers.end(), std::back_inserter(activeWorkers),
	             [](DownloadWorker *w) { return !w->IsGracefulStopping() && w->IsRunning(); });

	if (activeWorkers.empty()) {
		// During a thread-count reduction every worker may already be marked
		// for graceful shutdown. They still drain their queues, so use one
		// instead of dropping the request.
		std::copy_if(m_workers.begin(), m_workers.end(), std::back_inserter(activeWorkers),
		             [](DownloadWorker *w) { return w->IsRunning(); });
	}

	if (activeWorkers.empty()) return nullptr;

	const auto it = std::min_element(activeWorkers.begin(), activeWorkers.end(),
	                                 [](DownloadWorker *a, DownloadWorker *b) {
		                                 return a->activeDownloadCount() < b->activeDownloadCount();
	                                 });

	return *it;
}

void AsyncDownloadManager::enqueueDownload(WDownloadReplyBase *downloadable, const QUrl &url) {
	if (!downloadable) {
		return;
	}

	if (url.isEmpty()) {
		downloadable->discardData();
		emit downloadable->downloadFinished(QNetworkReply::ContentNotFoundError);
		return;
	}

	DownloadWorker *worker = nullptr;
	{
		QMutexLocker locker(&m_workersMutex);
		worker = getLeastBusyWorker();
		if (worker) {
			QMetaObject::invokeMethod(worker, "enqueueDownload",
			                          Q_ARG(WDownloadReplyBase*, downloadable),
			                          Q_ARG(QUrl, url));
		}
	}

	if (!worker) {
		qWarning() << "Failed to find an available worker";
		downloadable->discardData();
		emit downloadable->downloadFinished(QNetworkReply::OperationCanceledError);
	}
}

void AsyncDownloadManager::cleanupWorkers() {
	QVector<DownloadWorker *> workers;
	QVector<QThread *> threads;
	{
		QMutexLocker locker(&m_workersMutex);
		m_shuttingDown = true;
		workers = m_workers;
		threads = m_allThreads;
	}

	if (workers.isEmpty() && threads.isEmpty()) return;

	QEventLoop loop;
	QSet<DownloadWorker *> finished;
	auto markFinished = [&](DownloadWorker *worker) {
		if (finished.contains(worker)) {
			return;
		}
		finished.insert(worker);
		if (finished.size() >= workers.size()) {
			loop.quit();
		}
	};

	for (DownloadWorker *worker: workers) {
		// Drop the pool bookkeeping connection: it captures the raw thread
		// pointer and must not run after cleanupWorkers() deleted the thread.
		disconnect(worker, nullptr, this, nullptr);
		connect(worker, &DownloadWorker::stopped, &loop, [&, worker]() { markFinished(worker); });
		if (!worker->IsRunning()) {
			markFinished(worker);
		} else {
			QMetaObject::invokeMethod(worker, "stop", Qt::QueuedConnection);
		}
	}

	if (!workers.isEmpty() && finished.size() < workers.size()) {
		// Never hang application shutdown on a worker that is stuck in libcurl.
		QTimer::singleShot(5000, &loop, &QEventLoop::quit);
		loop.exec();
	}

	QVector<QThread *> threadsToDelete;
	for (QThread *thread: threads) {
		thread->quit();
		if (!thread->wait(5000)) {
			thread->terminate();
			if (!thread->wait(5000)) {
				// Deleting a running QThread would crash. Leaking it is the
				// lesser evil if the OS refuses to terminate the thread.
				qWarning() << "Download worker thread did not stop";
				continue;
			}
		}
		threadsToDelete.append(thread);
	}

	for (QThread *thread: threadsToDelete) {
		delete thread;
	}

	{
		QMutexLocker locker(&m_workersMutex);
		m_workers.clear();
		m_threads.clear();
		m_allThreads.clear();
		m_currentWorkerIndex = 0;
	}
}
