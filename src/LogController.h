#ifndef LOGCONTROLLER_H
#define LOGCONTROLLER_H

#include <QFile>
#include <QTextStream>
#include <QString>
#include <QObject>
#include <QMutex>
#include <QPointer>
#include <QMessageLogContext>

#include "pages/settings/Settings.h"

class LogController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString logPath READ logPath CONSTANT)
    Q_PROPERTY(QString logDir READ logDir)

public:
    static LogController &instance();

    void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

    void setSettings(Settings *settings) {
        QMutexLocker locker(&m_mutex);
        m_settings = settings;
    }

    QString logPath() const;

    QString logDir() const;

private:
    LogController(QObject *parent = nullptr);

    ~LogController();

    // Moves app.log to app.log.1 once it grows past the cap, so a long-lived
    // install does not accumulate an unbounded log file.
    void rotateIfNeeded();

    QPointer<Settings> m_settings;
    QFile m_logFile;
    QTextStream m_stream;
    QMutex m_mutex;
};


#endif //LOGCONTROLLER_H
