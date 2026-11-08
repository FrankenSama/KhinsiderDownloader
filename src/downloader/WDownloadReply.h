#ifndef WDOWNLOADREPLY_H
#define WDOWNLOADREPLY_H
#include <QDir>
#include <qfileinfo.h>
#include <QNetworkReply>
#include <QObject>
#include <atomic>

class WDownloadReplyBase : public QObject {
    Q_OBJECT

public:
    explicit WDownloadReplyBase(QObject *parent = nullptr) : QObject(parent),
                                                             maxRetryCount(3), currentRetryCount(0) {
    }

    virtual ~WDownloadReplyBase() {
    }

    void setMaxRetryCount(int count) { maxRetryCount = count; }
    int getMaxRetryCount() const { return maxRetryCount; }
    int getCurrentRetryCount() const { return currentRetryCount; }
    void incrementRetryCount() { currentRetryCount++; }
    void resetRetryCount() { currentRetryCount = 0; }

    bool isCanceled() const { return m_canceled.load(); }

    // A successful transfer must not be removed when the user cancels the
    // album after the file has already been written.
    bool isCompleted() const { return m_completed.load(); }
    void markCompleted() { m_completed.store(true); }

    QUrl url() const { return m_url; }
    void setURL(const QUrl &url) { m_url = url; }

    qint64 lastBytesReceived() const { return m_lastBytesReceived; }
    qint64 lastSpeedUpdate() const { return m_lastSpeedUpdate; }
    void setLastBytesRecieved(qint64 value) { m_lastBytesReceived = value; }
    void setLastSpeedUpdate(qint64 value) { m_lastSpeedUpdate = value; }

    virtual void appendData(const char *pData, size_t size) = 0;

    virtual void ensureContainerSize(qint64 minsize) = 0;

    virtual qint64 getDataSize() const = 0;

    // Empty the buffer/file so a retried request starts from scratch
    virtual void resetData() = 0;

    // Drop anything received so far for good, used when the response turned out to be an HTTP error
    virtual void discardData() = 0;

    // True when writing the received body failed (for example, the disk is full).
    virtual bool hasWriteError() const { return false; }

public slots:
    void cancel() { m_canceled.store(true); }
signals:
    void downloadFinished(QNetworkReply::NetworkError error = QNetworkReply::NoError);

    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);

    void downloadSpeed(qint64 bytesPerSecond);

    void downloadRetry();

protected:
    int maxRetryCount;
    int currentRetryCount;
    std::atomic<bool> m_canceled{false};
    std::atomic<bool> m_completed{false};
    QUrl m_url;
    qint64 m_lastBytesReceived = 0;
    qint64 m_lastSpeedUpdate = 0;
};

class WDownloadReplyMemory : public WDownloadReplyBase {
public:
    explicit WDownloadReplyMemory(QObject *parent = nullptr) : WDownloadReplyBase(parent) {
    }

    virtual ~WDownloadReplyMemory() {
    }

    void appendData(const char *pData, size_t size) override {
        m_data.append(pData, size);
    }

    void ensureContainerSize(qint64 minsize) override {
        if (m_data.capacity() < minsize && minsize > 0) {
            m_data.reserve(minsize);
        }
    }

    qint64 getDataSize() const override {
        return m_data.size();
    }

    void resetData() override {
        m_data.clear();
    }

    void discardData() override {
        m_data.clear();
    }

    // Get the data
    const QByteArray &getData() const { return m_data; }
    QByteArray &getData() { return m_data; }

private:
    QByteArray m_data;
};

class WDownloadReplyFile : public WDownloadReplyBase {
public:
    explicit WDownloadReplyFile(QObject *parent = nullptr) : WDownloadReplyBase(parent) {
    }

    virtual ~WDownloadReplyFile() {
        if (m_file.isOpen()) {
            m_file.close();
        }

        if (isCanceled() && !isCompleted()) {
            QFile::remove(m_file.fileName());
        }
    }

    bool setFilePath(const QString &filePath) {
        if (m_file.isOpen()) {
            m_file.close();
        }

        m_file.setFileName(filePath);
        m_writeError = false;

        QDir dir = QFileInfo(filePath).dir();
        if (!dir.exists()) {
            if (!dir.mkpath(".")) {
                qWarning() << "Failed to create directory in " << dir.path();
                return false;
            }
        }

        if (!m_file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            qWarning() << "Failed to open" << filePath << "for writing:" << m_file.errorString();
            return false;
        }

        return true;
    }

    void appendData(const char *pData, size_t size) override {
        if (!m_file.isOpen()) {
            m_writeError = true;
            return;
        }

        if (m_file.write(pData, static_cast<qint64>(size)) != static_cast<qint64>(size)) {
            m_writeError = true;
        }
    }

    bool hasWriteError() const override {
        return m_writeError;
    }

    void ensureContainerSize(qint64 minsize) override {
        Q_UNUSED(minsize);
    }

    qint64 getDataSize() const override {
        return m_file.size();
    }

    void resetData() override {
        m_writeError = false;
        if (m_file.isOpen()) {
            m_file.resize(0);
            m_file.seek(0);
        }
    }

    void discardData() override {
        if (m_file.isOpen()) {
            m_file.close();
        }
        QFile::remove(m_file.fileName());
    }

    QFile &getFile() { return m_file; }
    const QFile &getFile() const { return m_file; }

    QString filePath() const { return m_file.fileName(); }

private:
    QFile m_file;
    bool m_writeError = false;
};
#endif //WDOWNLOADREPLY_H
