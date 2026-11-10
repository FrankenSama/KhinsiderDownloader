#include "Settings.h"
#include <QDir>
#include <QStandardPaths>
#include "ImagePrecache.h"

void Settings::ParseFromConfig(QSettings &settings) {
    QString defaultDownloadPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    if (defaultDownloadPath.isEmpty()) {
        defaultDownloadPath = QDir::currentPath();
    }
    defaultDownloadPath += "/KhinsiderDownloader/";

    setDownloadPath(settings.value("downloadPath", defaultDownloadPath).toString());
    setDownloadThreads(settings.value("downloadThreads", 1).toInt());
    setMaxConcurrentDownloadsPerThread(settings.value("maxConcurrentDownloadsPerThread", 5).toInt());
    setPreferredAudioQualityInt(settings.value("preferredAudioQuality",
                                                static_cast<int>(DownloadQuality::MP3)).toInt());
    setEnableLogging(settings.value("enableLogging", true).toBool());
    setDownloadArt(settings.value("downloadArt", false).toBool());
    setSkipDownloaded(settings.value("skipDownloaded", true).toBool());

    m_windowWidth = qMax(settings.value("windowWidth", 1000).toInt(), 1000);
    m_windowHeight = qMax(settings.value("windowHeight", 600).toInt(), 600);
    m_lastTab = settings.value("lastTab", "searchtab").toString();
}

void Settings::saveWindowState(int width, int height, const QString &tab) {
    width = qMax(width, 1000);
    height = qMax(height, 600);

    bool changed = false;
    if (m_windowWidth != width) {
        m_windowWidth = width;
        changed = true;
    }
    if (m_windowHeight != height) {
        m_windowHeight = height;
        changed = true;
    }
    if (m_lastTab != tab) {
        m_lastTab = tab;
        changed = true;
    }

    if (changed) {
        emit windowStateChanged();
    }
}
