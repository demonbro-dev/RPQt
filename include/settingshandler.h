#ifndef SETTINGSHANDLER_H
#define SETTINGSHANDLER_H

#include <QObject>
#include <QSettings>
#include <QStandardPaths>

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
        RunAsClient,
        InstantPickByDefault,
        TopmostByDefault
    };
    enum StringConfigType {
        ServerHost,
        ServerPort
    };
    enum ListConfigType {
        Language
    };
    struct ConfigItem {
        QString section;
        QString key;
        QString name;
        QVariant defaultValue;
        QString type;
        QStringList options;
        QVariant enumValue;
    };

    explicit SettingsHandler(QObject *parent = nullptr);
    ~SettingsHandler();

    static bool generateExampleConfig();
    static QVector<ConfigItem> getConfigItems();

    bool getBoolConfig(BoolConfigType type) const;
    QString getStringConfig(StringConfigType type) const;
    QString getListConfig(ListConfigType type) const;

    QVariant getConfigValue(const QString &section, const QString &key) const;
    QVariant getConfigValue(const ConfigItem &item) const;

private:
    QSettings* m_settings;
};

#endif // SETTINGSHANDLER_H
