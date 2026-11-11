#include "Settings.h"
#include <QDir>
#include <QStandardPaths>
#include "ImagePrecache.h"

namespace {
QString defaultDownloadPath() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    if (path.isEmpty()) {
        path = QDir::currentPath();
    }
    return path + "/KhinsiderDownloader/";
}
}

void Settings::ParseFromConfig(QSettings &settings) {
    setDownloadPath(settings.value("downloadPath", defaultDownloadPath()).toString());
    setDownloadThreads(settings.value("downloadThreads", 1).toInt());
    setMaxConcurrentDownloadsPerThread(settings.value("maxConcurrentDownloadsPerThread", 5).toInt());
    setPreferredAudioQualityInt(settings.value("preferredAudioQuality",
                                                static_cast<int>(DownloadQuality::MP3)).toInt());
    setEnableLogging(settings.value("enableLogging", true).toBool());
    setDownloadArt(settings.value("downloadArt", false).toBool());
    setSkipDownloaded(settings.value("skipDownloaded", true).toBool());

    m_windowWidth = qMax(settings.value("windowWidth", 1000).toInt(), 1000);
    m_windowHeight = qMax(settings.value("windowHeight", 600).toInt(), 600);
    m_windowPositionValid = settings.contains("windowX") && settings.contains("windowY");
    m_windowX = settings.value("windowX", 0).toInt();
    m_windowY = settings.value("windowY", 0).toInt();
    m_windowMaximized = settings.value("windowMaximized", false).toBool();
    m_lastTab = settings.value("lastTab", "searchtab").toString();
}

void Settings::saveWindowState(int width, int height, int x, int y, bool maximized,
                               const QString &tab) {
    width = qMax(width, 1000);
    height = qMax(height, 600);

    const bool hadPosition = m_windowPositionValid;

    bool changed = false;
    if (m_windowWidth != width) {
        m_windowWidth = width;
        changed = true;
    }
    if (m_windowHeight != height) {
        m_windowHeight = height;
        changed = true;
    }
    if (!hadPosition || m_windowX != x) {
        m_windowX = x;
        changed = true;
    }
    if (!hadPosition || m_windowY != y) {
        m_windowY = y;
        changed = true;
    }
    if (!hadPosition) {
        m_windowPositionValid = true;
        changed = true;
    }
    if (m_windowMaximized != maximized) {
        m_windowMaximized = maximized;
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

void Settings::resetToDefaults() {
    setDownloadPath(defaultDownloadPath());
    setDownloadThreads(1);
    setMaxConcurrentDownloadsPerThread(5);
    setPreferredAudioQuality(DownloadQuality::MP3);
    setEnableLogging(true);
    setDownloadArt(false);
    setSkipDownloaded(true);
}
