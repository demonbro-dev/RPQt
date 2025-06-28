#ifndef SETTINGSHANDLER_H
#define SETTINGSHANDLER_H

#include <QObject>
#include <QSettings>

class SettingsHandler : public QObject
{
    Q_OBJECT

public:
    explicit SettingsHandler(QObject *parent = nullptr);
    ~SettingsHandler();

    static bool generateExampleConfig();

    bool getOpenRandMirageWhenClose() const;
    bool getUseLightTheme() const;

private:
    QSettings* m_settings;
    bool m_openRandMirageWhenClose;
    bool m_useLightTheme;
};

#endif // SETTINGSHANDLER_H
