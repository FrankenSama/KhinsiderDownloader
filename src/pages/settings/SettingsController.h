#ifndef SETTINGSCONTROLLER_H
#define SETTINGSCONTROLLER_H
#include <QObject>
#include <QString>
#include <QSettings>

#include "Settings.h"
#include "SettingsViewModel.h"

class SettingsController : public QObject {
    Q_OBJECT
    Q_PROPERTY(SettingsViewModel* settingsVM READ settingsVM CONSTANT)

public:
    explicit SettingsController(QObject *parent = nullptr, Settings *settings = nullptr);

public:
    SettingsViewModel *settingsVM() { return m_settingsVM; }

public:
    // Opens the folder that contains config.ini in the system file browser.
    Q_INVOKABLE void openConfigFolder();

    // Restores the preference values to their defaults and saves them.
    Q_INVOKABLE void resetToDefaults();

public slots:
    void saveConfig();

private:
    SettingsViewModel *m_settingsVM;
    QSettings m_settings;
    Settings *m_SettingsObject;
};


#endif //SETTINGSCONTROLLER_H
