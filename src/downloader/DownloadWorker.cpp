#include "DownloadWorker.h"
#include <QDebug>
#include <QDateTime>
#include <QVector>
#include "QTimer"
#include "QUrl"
#include <QtNetwork/QNetworkReply>

DownloadWorker::DownloadWorker(int id, int maxConcurrentDownloads, QObject *parent)
	: QObject(parent), m_id(id), m_maxConcurrentDownloads(maxConcurrentDownloads) {
	m_multiHandle = curl_multi_init();
	static_assert(std::is_same<decltype(m_activeDownloads)::key_type, CURL *>::value,
	              "m_activeDownloads key type must be CURL*");
	static_assert(std::is_same<decltype(m_activeDownloads)::mapped_type, WDownloadReplyBase *>::value,
	              "m_activeDownloads value type must be WDownloadReplyBase*");
}

DownloadWorker::~DownloadWorker() {
	stop();

	if (m_multiHandle) {
		curl_multi_cleanup(m_multiHandle);
		m_multiHandle = nullptr;
	}

	delete m_multiTimer;
	delete m_queueTimer;
}

void DownloadWorker::enqueueDownload(WDownloadReplyBase *downloadable, const QUrl &url) {
	if (!downloadable) {
		return;
	}

	// A worker that is gracefully stopping still drains the queue it already
	// accepted; only a fully stopped worker rejects new work.
	if (!m_running.load()) {
		downloadable->discardData();
		emit downloadable->downloadFinished(QNetworkReply::OperationCanceledError);
		return;
	}

	qDebug() << "Worker" << m_id << "- Queuing download from:" << url.toString();
	QMutexLocker locker(&m_queueMutex);
	downloadable->setURL(url);
	m_queue.enqueue(downloadable);
	m_queueCondition.wakeOne();

	//kick queue :)
	QMetaObject::invokeMethod(this, "checkQueue", Qt::QueuedConnection);
}

void DownloadWorker::stop() {
	// stop() can be reached from the queued shutdown request, the graceful
	// shutdown path and the destructor. Only the first call must do the work.
	bool expected = false;
	if (!m_stopped.compare_exchange_strong(expected, true)) {
		return;
	}

	m_running.store(false);
	m_queueCondition.wakeAll();

	// Cancel everything that is still waiting so callers do not hang forever.
	QQueue<WDownloadReplyBase *> pendingQueue;
	QSet<WDownloadReplyBase *> pendingRetries;
	{
		QMutexLocker locker(&m_queueMutex);
		pendingQueue.swap(m_queue);
		pendingRetries.swap(m_pendingRetries);
	}
	for (WDownloadReplyBase *pending: pendingQueue) {
		if (pending) {
			pending->discardData();
			emit pending->downloadFinished(QNetworkReply::OperationCanceledError);
		}
	}
	for (WDownloadReplyBase *pending: pendingRetries) {
		if (pending) {
			pending->discardData();
			emit pending->downloadFinished(QNetworkReply::OperationCanceledError);
		}
	}

	for (auto it = m_activeDownloads.begin(); it != m_activeDownloads.end(); ++it) {
		if (it.value()) {
			it.value()->discardData();
		}
		if (m_multiHandle) {
			curl_multi_remove_handle(m_multiHandle, it.key());
		}
		curl_easy_cleanup(it.key());
	}
	m_activeDownloads.clear();
	m_activeDownloadCount.store(0);

	emit stopped();
}

void DownloadWorker::start() {
	if (m_multiTimer) {
		return;
	}

	m_multiTimer = new QTimer(this);
	connect(m_multiTimer, &QTimer::timeout, this, &DownloadWorker::checkMultiInfo);
	m_multiTimer->start(m_sleepingMultiInfoTimerInterval);

	m_queueTimer = new QTimer(this);
	connect(m_queueTimer, &QTimer::timeout, this, &DownloadWorker::checkQueue);
	m_queueTimer->start(m_queueTimerInterval);
}

void DownloadWorker::checkQueue() {
	if (!m_running.load()) return;

	{
		QMutexLocker locker(&m_maxConCurrentDownloadsMutex);
		if (m_maxConcurrentDownloads > 0 &&
		    m_activeDownloadCount.load() >= m_maxConcurrentDownloads) {
			return;
		}
	}

	WDownloadReplyBase *request = nullptr;
	QVector<WDownloadReplyBase *> canceledRequests;
	{
		QMutexLocker locker(&m_queueMutex);

		while (!m_queue.isEmpty()) {
			request = m_queue.head();
			if (request && request->isCanceled()) {
				m_queue.dequeue();
				canceledRequests.append(request);
				request = nullptr;
				continue;
			}
			break;
		}

		if (m_queue.isEmpty()) {
			request = nullptr;
		} else {
			request = m_queue.dequeue();
		}
	}

	// Emit after releasing m_queueMutex: a directly connected slot is allowed
	// to call back into activeDownloadCount() without deadlocking.
	for (WDownloadReplyBase *canceled: canceledRequests) {
		emit canceled->downloadFinished(QNetworkReply::OperationCanceledError);
	}

	if (request) {
		startDownload(request);
	}
}

int DownloadWorker::activeDownloadCount() const {
	QMutexLocker locker(&m_queueMutex);
	return m_queue.size() + m_pendingRetries.size() + m_activeDownloadCount.load();
}

void DownloadWorker::setMaxConcurrentDownloads(int nMaxDownloads) {
	QMutexLocker locker(&m_maxConCurrentDownloadsMutex);
	// 0 means unlimited, negative values are invalid.
	m_maxConcurrentDownloads = qMax(0, nMaxDownloads);
}

void DownloadWorker::checkMultiInfo() {
	if (!m_running.load() || !m_multiHandle || !m_multiTimer) {
		return;
	}

	if (m_gracefulStopping.load() && m_activeDownloads.empty()) {
		bool workPending = false;
		{
			QMutexLocker locker(&m_queueMutex);
			workPending = !m_queue.isEmpty() || !m_pendingRetries.isEmpty();
		}
		if (!workPending) {
			stop();
			return;
		}
	}

	int stillRunning = 0;
	CURLMcode mc;
	do {
		mc = curl_multi_perform(m_multiHandle, &stillRunning);
	} while (mc == CURLM_CALL_MULTI_PERFORM);

	if (mc != CURLM_OK) {
		qWarning() << "curl_multi_perform failed with error:" << curl_multi_strerror(mc);
		return;
	}

	CURLMsg *msg;
	int msgsLeft;

	while ((msg = curl_multi_info_read(m_multiHandle, &msgsLeft))) {
		if (msg->msg == CURLMSG_DONE) {
			CURL *easyHandle = msg->easy_handle;
			CURLcode result = msg->data.result;

			if (m_activeDownloads.contains(easyHandle)) {
				WDownloadReplyBase *downloadable = m_activeDownloads.value(easyHandle);

				// The status has to be read before the handle is cleaned up
				long httpStatus = 0;
				curl_easy_getinfo(easyHandle, CURLINFO_RESPONSE_CODE, &httpStatus);

				curl_multi_remove_handle(m_multiHandle, easyHandle);
				curl_easy_cleanup(easyHandle);

				handleDownloadFinished(downloadable, result, httpStatus);
				//queue it's time to check itself!
				QMetaObject::invokeMethod(this, "checkQueue", Qt::QueuedConnection);
			}
		}
	}

	if (stillRunning > 0) {
		//We still have to process some data, use working interval
		m_multiTimer->start(m_activeMultiInfoTimerInterval);
	} else if (m_multiTimer->interval() != m_sleepingMultiInfoTimerInterval) {
		//We're empty, it's time to relax
		m_multiTimer->start(m_sleepingMultiInfoTimerInterval);
	}
	checkCancellations();
}

void DownloadWorker::startDownload(WDownloadReplyBase *request) {
	if (!request) {
		return;
	}

	if (request->url().isEmpty()) {
		request->discardData();
		emit request->downloadFinished(QNetworkReply::ContentNotFoundError);
		return;
	}

	if (request->isCanceled()) {
		request->discardData();
		emit request->downloadFinished(QNetworkReply::OperationCanceledError);
		return;
	}

	qDebug() << "Worker" << m_id << "- Starting download from:" << request->url().toString();

	CURL *easyHandle = curl_easy_init();
	if (!easyHandle) {
		request->discardData();
		emit request->downloadFinished(QNetworkReply::UnknownNetworkError);
		return;
	}

	// Create download data structure
	request->setLastBytesRecieved(0);
	request->setLastSpeedUpdate(QDateTime::currentMSecsSinceEpoch());

	// Set options
	curl_easy_setopt(easyHandle, CURLOPT_URL, request->url().toEncoded().constData());
	curl_easy_setopt(easyHandle, CURLOPT_WRITEFUNCTION, writeCallback);
	curl_easy_setopt(easyHandle, CURLOPT_WRITEDATA, request);
	curl_easy_setopt(easyHandle, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(easyHandle, CURLOPT_PRIVATE, request);
	curl_easy_setopt(easyHandle, CURLOPT_NOSIGNAL, 1L);
	// A fixed total timeout kills large album tracks on slow connections.
	// Bound connection setup instead, and abort only if the transfer stalls.
	curl_easy_setopt(easyHandle, CURLOPT_CONNECTTIMEOUT, 30L);
	curl_easy_setopt(easyHandle, CURLOPT_LOW_SPEED_LIMIT, 1024L);
	curl_easy_setopt(easyHandle, CURLOPT_LOW_SPEED_TIME, 60L);
	curl_easy_setopt(easyHandle, CURLOPT_BUFFERSIZE, CURL_MAX_READ_SIZE); // 2MB buffer
	curl_easy_setopt(easyHandle, CURLOPT_TCP_NODELAY, 1L);
	curl_easy_setopt(easyHandle, CURLOPT_TCP_KEEPALIVE, 1L);
	curl_easy_setopt(easyHandle, CURLOPT_TCP_FASTOPEN, 1L);
	curl_easy_setopt(easyHandle, CURLOPT_PIPEWAIT, 1L);
    //curl_easy_setopt(easyHandle, CURLOPT_SSL_VERIFYPEER, 0L);
	// Set up progress callback
	curl_easy_setopt(easyHandle, CURLOPT_XFERINFOFUNCTION, progressCallback);
	curl_easy_setopt(easyHandle, CURLOPT_XFERINFODATA, request);
	curl_easy_setopt(easyHandle, CURLOPT_NOPROGRESS, 0L);

	// Add to multi handle
	CURLMcode mc = curl_multi_add_handle(m_multiHandle, easyHandle);
	if (mc != CURLM_OK) {
		request->discardData();
		emit request->downloadFinished(QNetworkReply::UnknownNetworkError);
		curl_easy_cleanup(easyHandle);
		return;
	}

	// Store mappings
	m_activeDownloads.insert(easyHandle, request);
	m_activeDownloadCount.fetch_add(1);
	qDebug() << "Inserting... " << QThread::currentThreadId();
	//kick multitimer, it has some work
	QMetaObject::invokeMethod(this, "checkMultiInfo", Qt::QueuedConnection);
}

size_t DownloadWorker::writeCallback(char *ptr, size_t size, size_t nmemb, void *userdata) {
	auto data = static_cast<WDownloadReplyBase *>(userdata);
	if (!data) {
		return 0;
	}

	size_t bytesToWrite = size * nmemb;
	data->appendData(ptr, bytesToWrite);

	if (data->hasWriteError()) {
		// Abort the transfer so a full disk does not produce a truncated file
		// that later looks like a successful download.
		return 0;
	}

	return bytesToWrite;
}

void DownloadWorker::checkCancellations() {
	//bool bSomethingCanceled = false;
	//for (auto it = m_activeDownloads.begin(); it != m_activeDownloads.end(); ++it) {
	//	if (it.value()->isCanceled()) {
	//		curl_easy_setopt(it.key(), CURLOPT_TIMEOUT_MS, 1);
	//		bSomethingCanceled = true;
	//	}
	//}
	//

	//if (bSomethingCanceled) {
	//	QMetaObject::invokeMethod(this, "checkMultiInfo", Qt::QueuedConnection);
	//}
}

int DownloadWorker::progressCallback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal,
                                     curl_off_t ulnow) {
	Q_UNUSED(ultotal);
	Q_UNUSED(ulnow);

	auto data = static_cast<WDownloadReplyBase *>(clientp);
	if (!data) {
		return 1;
	}

	if (data->isCanceled()) {
		return 1;
	}

	emit data->downloadProgress(dlnow, dltotal);
	data->ensureContainerSize(dltotal);
	qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
	qint64 elapsed = currentTime - data->lastSpeedUpdate();

	if (elapsed >= 1000) {
		// update speed every second
		qint64 bytesReceived = dlnow - data->lastBytesReceived();
		qint64 bytesPerSecond = (bytesReceived * 1000) / elapsed;
		if (bytesPerSecond != 0) {
			emit data->downloadSpeed(bytesPerSecond);
		}
		data->setLastBytesRecieved(dlnow);
		data->setLastSpeedUpdate(currentTime);
	}

	return 0;
}

void DownloadWorker::handleDownloadFinished(WDownloadReplyBase *downloadable, CURLcode result, long httpStatus) {
	if (!downloadable || !m_activeDownloads.values().contains(downloadable)) {
		return;
	}

	CURL *easyHandle = nullptr;
	for (auto it = m_activeDownloads.begin(); it != m_activeDownloads.end(); ++it) {
		if (it.value() == downloadable) {
			easyHandle = it.key();
			break;
		}
	}
	qDebug() << "Active Downloads" << m_activeDownloads.size();
	// Find both the handle and data

	if (!easyHandle) {
		return; // Should not happen
	}

	if (result == CURLE_OK && httpStatus >= 400) {
		// libcurl reports a transfer that returned an error page as a success.
		// Don't hand that body to the parsers or leave it behind as a file.
		qWarning() << "Worker" << m_id << "- HTTP" << httpStatus << "from:" << downloadable->url().toString();

		if (downloadable->isCanceled()) {
			downloadable->discardData();
			emit downloadable->downloadFinished(QNetworkReply::OperationCanceledError);
		} else if (isRetryableHttpStatus(httpStatus) && downloadable->getCurrentRetryCount() < downloadable->getMaxRetryCount()) {
			downloadable->incrementRetryCount();
			emit downloadable->downloadRetry();

			downloadable->resetData();
			downloadable->setLastBytesRecieved(0);
			scheduleRetry(downloadable);
		} else {
			downloadable->discardData();
			emit downloadable->downloadFinished(mapHttpStatusToNetworkError(httpStatus));
		}
	} else if (result == CURLE_OK) {
		qDebug() << "Worker" << m_id << "- Download successful from:" << downloadable->url().toString();

		downloadable->markCompleted();

		// Done, emit finished
		emit downloadable->downloadFinished(QNetworkReply::NoError);
	} else if (downloadable->isCanceled()) {
		downloadable->discardData();
		emit downloadable->downloadFinished(QNetworkReply::OperationCanceledError);
	} else {
		qDebug() << "Worker" << m_id << "- Download failed from:" << downloadable->url().toString()
				<< "with error:" << curl_easy_strerror(result);

		QNetworkReply::NetworkError networkError = mapCurlErrorToNetworkError(result);

		// A write failure (full disk, removed drive) will not fix itself on
		// retry, and retrying would just overwrite the same broken file.
		if (downloadable->hasWriteError()) {
			downloadable->discardData();
			emit downloadable->downloadFinished(networkError);
		} else if (downloadable->getCurrentRetryCount() < downloadable->getMaxRetryCount()) {
			downloadable->incrementRetryCount();
			emit downloadable->downloadRetry();

			// Re-queue for download, starting from an empty buffer/file so the body isn't appended twice
			downloadable->resetData();
			downloadable->setLastBytesRecieved(0);

			scheduleRetry(downloadable);
		} else {
			// Never leave a partial file behind after the final failure.
			downloadable->discardData();
			emit downloadable->downloadFinished(networkError);
		}
	}
	m_activeDownloads.remove(easyHandle);
	m_activeDownloadCount.fetch_sub(1);
}

void DownloadWorker::scheduleRetry(WDownloadReplyBase *downloadable) {
	if (!downloadable)
		return;

	{
		QMutexLocker locker(&m_queueMutex);
		m_pendingRetries.insert(downloadable);
	}

	QTimer *retryTimer = new QTimer(this);
	retryTimer->setSingleShot(true);
	connect(retryTimer, &QTimer::timeout, [this, downloadable, retryTimer]() {
		retryTimer->deleteLater();

		{
			QMutexLocker locker(&m_queueMutex);
			if (!m_pendingRetries.remove(downloadable)) {
				// stop() already reported this request as canceled.
				return;
			}
		}

		if (!m_running.load()) {
			return;
		}

		QMutexLocker locker(&m_queueMutex);
		m_queue.enqueue(downloadable);
		m_queueCondition.wakeOne();

		// Invoke checkQueue to process the retry immediately
		QMetaObject::invokeMethod(this, "checkQueue", Qt::QueuedConnection);
	});

	qDebug() << "Worker" << m_id << "- Scheduling retry for:" << downloadable->url().toString()
			<< "in" << m_retryInvetrval << "ms";

	retryTimer->start(m_retryInvetrval);
}

bool DownloadWorker::isRetryableHttpStatus(long httpStatus) {
	// 403 is included on purpose: the site's bot protection answers an otherwise valid request
	// with a 403 every now and then, and the same request passes a moment later.
	return httpStatus == 403 || httpStatus == 408 || httpStatus == 429 || httpStatus >= 500;
}

QNetworkReply::NetworkError DownloadWorker::mapHttpStatusToNetworkError(long httpStatus) {
	switch (httpStatus) {
		case 401:
			return QNetworkReply::AuthenticationRequiredError;
		case 403:
			return QNetworkReply::ContentAccessDenied;
		case 404:
		case 410:
			return QNetworkReply::ContentNotFoundError;
		case 503:
			return QNetworkReply::ServiceUnavailableError;
		default:
			return httpStatus >= 500 ? QNetworkReply::InternalServerError : QNetworkReply::UnknownContentError;
	}
}

QNetworkReply::NetworkError DownloadWorker::mapCurlErrorToNetworkError(CURLcode curlError) {
	switch (curlError) {
		case CURLE_COULDNT_RESOLVE_HOST:
			return QNetworkReply::HostNotFoundError;
		case CURLE_COULDNT_CONNECT:
			return QNetworkReply::ConnectionRefusedError;
		case CURLE_OPERATION_TIMEDOUT:
			return QNetworkReply::TimeoutError;
		case CURLE_SSL_CONNECT_ERROR:
			return QNetworkReply::SslHandshakeFailedError;
		case CURLE_GOT_NOTHING:
		case CURLE_RECV_ERROR:
			return QNetworkReply::RemoteHostClosedError;
		case CURLE_ABORTED_BY_CALLBACK:
			return QNetworkReply::OperationCanceledError;
		default:
			return QNetworkReply::UnknownNetworkError;
	}
}
