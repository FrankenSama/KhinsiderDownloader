#ifndef ABOUTCONTROLLER_H
#define ABOUTCONTROLLER_H
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>

class AboutController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString appVersion READ appVersion NOTIFY appVersionChanged)

public:
    explicit AboutController(QObject *parent = nullptr);
signals:
    void foundNewUpdate(const QString &version);
    // Result of a check when no update is available or GitHub can't be
    // reached; the About page uses these for its inline status line.
    void upToDate();
    void updateCheckFailed();
public slots:
    void onUpdateCheckFinished(QNetworkReply *reply);
    void checkForUpdates();
    QString appVersion() const;
signals:
    void appVersionChanged();
private:
    bool isNewVersion(const QString &latestVersion, const QString &currentVersion);

private:
    QString m_version;
    QNetworkAccessManager *m_manager = nullptr;
};
#endif // ABOUTCONTROLLER_H
