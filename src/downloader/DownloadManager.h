#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QObject>
#include <QVector>
#include <QThread>
#include "DownloadWorker.h"

class AsyncDownloadManager : public QObject {
    Q_OBJECT

public:
    explicit AsyncDownloadManager(QObject *parent = nullptr);

    ~AsyncDownloadManager();

    void setDownloadLimitPerThread(int limit);

    void setThreadCount(int count);

    void reduceWorkers();

    void addWorkers();

    void enqueueDownload(WDownloadReplyBase *downloadable, const QUrl &url);

private:
    DownloadWorker *getLeastBusyWorker();

private:
    int m_limitPerThread;
    int m_threadCount;

    QVector<QThread *> m_threads;
    QVector<QThread *> m_allThreads;
    QVector<DownloadWorker *> m_workers;

    int m_currentWorkerIndex;
    int m_nextWorkerId = 0;
    bool m_shuttingDown = false;

    void cleanupWorkers();

    QMutex m_workersMutex;
};
#endif
