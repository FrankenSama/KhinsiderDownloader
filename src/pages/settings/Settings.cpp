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
}
