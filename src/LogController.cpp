#include "LogController.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QDebug>
#include <QDesktopServices>
#include <QStandardPaths>

namespace {
constexpr qint64 MaxLogSize = 5 * 1024 * 1024;
}

LogController &LogController::instance() {
    static LogController instance;
    return instance;
}

LogController::LogController(QObject *parent)
    : QObject(parent) {
    const QString logDirectory = logDir();
    QDir().mkpath(logDirectory);

    m_logFile.setFileName(logDirectory + "app.log");
    const bool logFileOpened = m_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    m_stream.setDevice(logFileOpened ? &m_logFile : nullptr);
    rotateIfNeeded();
}

LogController::~LogController() {
    m_logFile.close();
}

void LogController::messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    QString level;
    switch (type) {
        case QtDebugMsg: level = "DEBUG";
            break;
        case QtInfoMsg: level = "INFO";
            break;
        case QtWarningMsg: level = "WARNING";
            break;
        case QtCriticalMsg: level = "CRITICAL";
            break;
        case QtFatalMsg: level = "FATAL";
            break;
    }

    QString logMsg = QString("[%1] %2: %3 (%4:%5, %6)\n")
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"))
            .arg(level)
            .arg(msg)
            .arg(context.file)
            .arg(context.line)
            .arg(context.function);

    {
        QMutexLocker locker(&m_mutex);
        if (m_settings && m_settings->enableLogging() && m_logFile.isOpen()) {
            m_stream << logMsg << Qt::flush;
            rotateIfNeeded();
        }
    }
#ifdef QT_DEBUG
    QTextStream(stderr) << logMsg;
    //fprintf(stderr, "%s", logMsg.toLocal8Bit().constData());
#endif
}

void LogController::rotateIfNeeded() {
    if (!m_logFile.isOpen() || m_logFile.size() <= MaxLogSize) {
        return;
    }

    const QString path = m_logFile.fileName();
    m_logFile.close();

    const QString backupPath = path + ".1";
    QFile::remove(backupPath);
    if (!QFile::rename(path, backupPath)) {
        // If the rename fails, keep appending rather than dropping logs.
        if (!m_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            m_stream.setDevice(nullptr);
        }
        return;
    }

    m_logFile.setFileName(path);
    if (m_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        m_stream.setDevice(&m_logFile);
    } else {
        m_stream.setDevice(nullptr);
    }
}

QString LogController::logPath() const {
    return m_logFile.fileName();
}

QString LogController::logDir() const {
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    if (path.isEmpty()) {
        path = QCoreApplication::applicationDirPath();
    }
    return path + "/logs/";
}
