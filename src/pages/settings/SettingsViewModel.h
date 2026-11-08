#ifndef SETTINGSVIEWMODEL_H
#define SETTINGSVIEWMODEL_H
#include "Settings.h"

class SettingsViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(Settings* settings READ settings NOTIFY settingsChanged)

public:
    explicit SettingsViewModel(QObject *parent = nullptr, Settings *settings = nullptr)
        : QObject(parent) {
        setSettingsObject(settings);
    }

    Settings *settings() { return m_Settings; }

public slots:
    void setSettingsObject(Settings *settings) {
        m_Settings = settings;
        emit settingsChanged();
    }

signals:
    void settingsChanged();

private:
    Settings *m_Settings = nullptr;
};
#endif //SETTINGSVIEWMODEL_H
