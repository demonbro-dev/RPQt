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
        UseLightTheme,
        RunAsClient
    };
    enum StringConfigType {
        ServerHost,
        ServerPort
    };

    explicit SettingsHandler(QObject *parent = nullptr);
    ~SettingsHandler();

    static bool generateExampleConfig();

    bool getBoolConfig(BoolConfigType type) const;
    QString getStringConfig(StringConfigType type) const;

private:
    QSettings* m_settings;
    bool m_openRandMirageWhenClose;
    bool m_useLightTheme;
    bool m_runAsClient;
};

#endif // SETTINGSHANDLER_H
