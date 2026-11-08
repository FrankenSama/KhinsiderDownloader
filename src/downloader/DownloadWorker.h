#ifndef DOWNLOADWORKER_H
#define DOWNLOADWORKER_H

#include <QQueue>
#include <QSet>
#include <QMutex>
#include <QWaitCondition>
#include <QFile>
#include <QThread>
#include <atomic>
#include <curl/curl.h>
#include <qtimer.h>

#include "WDownloadReply.h"


class DownloadWorker : public QObject {
	Q_OBJECT

public:
	explicit DownloadWorker(int id, int maxConcurrentDownloads, QObject *parent = nullptr);

	~DownloadWorker() override;

public slots:
	void enqueueDownload(WDownloadReplyBase *downloadable, const QUrl &url);

	void stop();

	void start();

private slots:
	void checkMultiInfo();

	void handleDownloadFinished(WDownloadReplyBase *downloadable, CURLcode result, long httpStatus);

	void checkCancellations();

	void checkQueue();

	void prepareForShutdown() { m_gracefulStopping.store(true); }
signals:
	void stopped();

public:
	int activeDownloadCount() const;

	void setMaxConcurrentDownloads(int nMaxDownloads);

	bool IsGracefulStopping() const { return m_gracefulStopping.load(); }
	bool IsRunning() const { return m_running.load(); }

	void scheduleRetry(WDownloadReplyBase *downloadable);

	void setRetryInterval(int milliseconds) {
		m_retryInvetrval = milliseconds;
	}

	int retryInterval() const {
		return m_retryInvetrval;
	}

private:
	int m_activeMultiInfoTimerInterval = 50;
	int m_sleepingMultiInfoTimerInterval = 1000;
	int m_queueTimerInterval = 1000;
	int m_id;
	int m_maxConcurrentDownloads;
	std::atomic<bool> m_running{true};
	std::atomic<bool> m_stopped{false};
	int m_retryInvetrval = 5000;
	//it means that we don't accept new orders in queue and will self destroy after finishing download
	std::atomic<bool> m_gracefulStopping{false};

	QQueue<WDownloadReplyBase *> m_queue;
	QSet<WDownloadReplyBase *> m_pendingRetries;
	mutable QMutex m_queueMutex;
	QWaitCondition m_queueCondition;
	QTimer *m_multiTimer = nullptr;
	QTimer *m_queueTimer = nullptr;
	QMutex m_maxConCurrentDownloadsMutex;
	CURLM *m_multiHandle = nullptr;
	QHash<CURL *, WDownloadReplyBase *> m_activeDownloads;
	std::atomic<int> m_activeDownloadCount{0};

	void startDownload(WDownloadReplyBase *request);

	static size_t writeCallback(char *ptr, size_t size, size_t nmemb, void *userdata);


	static int progressCallback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal,
	                            curl_off_t ulnow);

	QNetworkReply::NetworkError mapCurlErrorToNetworkError(CURLcode curlError);
	static QNetworkReply::NetworkError mapHttpStatusToNetworkError(long httpStatus);
	static bool isRetryableHttpStatus(long httpStatus);
};
#endif
