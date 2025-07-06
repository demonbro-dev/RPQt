#ifndef SETTINGSHANDLER_H
#define SETTINGSHANDLER_H

#include <QObject>
#include <QSettings>

#ifdef Q_OS_WIN
#define CONFIG_PATH QCoreApplication::applicationDirPath() + "/config.ini"
#else
#define CONFIG_PATH QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/RPconfig.ini"
#endif

class SettingsHandler : public QObject
{
    Q_OBJECT

public:
    enum BoolConfigType {
        OpenRandMirageWhenClose,
        UseLightTheme
    };

    explicit SettingsHandler(QObject *parent = nullptr);
    ~SettingsHandler();

    static bool generateExampleConfig();

    bool getBoolConfig(BoolConfigType type) const;

private:
    QSettings* m_settings;
    bool m_openRandMirageWhenClose;
    bool m_useLightTheme;
};

#endif // SETTINGSHANDLER_H
