#include "SettingsController.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

namespace {
QString configFilePath() {
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    if (configDir.isEmpty()) {
        configDir = QDir::currentPath();
    }
    QDir().mkpath(configDir);

    const QString configPath = configDir + "/config.ini";
    const QString legacyPath = QDir::currentPath() + "/config.ini";

    // Migrate the old working-directory config once, so existing users keep
    // their download path and quality settings.
    if (!QFileInfo::exists(configPath) && QFileInfo::exists(legacyPath)) {
        if (!QFile::rename(legacyPath, configPath)) {
            if (QFile::copy(legacyPath, configPath)) {
                QFile::remove(legacyPath);
            }
        }
    }

    return configPath;
}
}

SettingsController::SettingsController(QObject *parent, Settings *settings): QObject(parent),
                                                                             m_settings(configFilePath(),
                                                                                 QSettings::IniFormat) {
    QSettings::setDefaultFormat(QSettings::IniFormat);
    m_SettingsObject = settings;
    if (m_SettingsObject) {
        m_SettingsObject->ParseFromConfig(m_settings);
    }
    m_settingsVM = new SettingsViewModel(parent);
    m_settingsVM->setSettingsObject(m_SettingsObject);
    if (m_SettingsObject) {
        connect(m_SettingsObject, &Settings::settingsChanged, m_settingsVM, &SettingsViewModel::settingsChanged);
        connect(m_SettingsObject, &Settings::settingsChanged, this, &SettingsController::saveConfig);
        connect(m_SettingsObject, &Settings::windowStateChanged, this, &SettingsController::saveConfig);
    }
}

void SettingsController::saveConfig() {
    if (!m_SettingsObject) {
        return;
    }

    m_settings.setValue("downloadPath", m_SettingsObject->downloadPath());
    m_settings.setValue("enableLogging", m_SettingsObject->enableLogging());
    m_settings.setValue("downloadThreads", m_SettingsObject->downloadThreads());
    m_settings.setValue("preferredAudioQuality", static_cast<int>(m_SettingsObject->preferredAudioQuality()));
    m_settings.setValue("maxConcurrentDownloadsPerThread",
                        static_cast<int>(m_SettingsObject->maxConcurrentDownloadsPerThread()));
    m_settings.setValue("downloadArt", static_cast<bool>(m_SettingsObject->downloadArt()));
    m_settings.setValue("skipDownloaded", static_cast<bool>(m_SettingsObject->skipDownloaded()));
    m_settings.setValue("windowWidth", m_SettingsObject->windowWidth());
    m_settings.setValue("windowHeight", m_SettingsObject->windowHeight());
    m_settings.setValue("lastTab", m_SettingsObject->lastTab());
    m_settings.sync();
}
