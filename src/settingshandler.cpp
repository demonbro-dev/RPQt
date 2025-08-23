#include "settingshandler.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>

SettingsHandler::SettingsHandler(QObject *parent) : QObject(parent)
{
    QString configPath = CONFIG_PATH;
    if (QFile::exists(configPath)) {
        m_settings = new QSettings(configPath, QSettings::IniFormat, this);
    } else {
        m_settings = nullptr;
    };
}

SettingsHandler::~SettingsHandler()
{
    delete m_settings;
}

bool SettingsHandler::generateExampleConfig()
{
    QString configPath = CONFIG_PATH;
    QFile file(configPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    out << "# Template configuration file for RandPicker\n";
    out << "# All lines starting with '#' are comments and will be ignored.\n";
    out << "# To enable a setting,remove the '#' ahead of it.\n";
    out << "\n";
    out << "[Window]\n";
    out << "#OpenRandMirageWhenClose=false\n";
    out << "#UseLightTheme=false\n";
    out << "\n";
    out << "[RPWeb]\n";
    out << "#RunAsClient=false\n";
    out << "#Server=127.0.0.1\n";
    out << "#Port=8080";

    file.close();
    return true;
}

bool SettingsHandler::getBoolConfig(BoolConfigType type) const
{
    if (!m_settings) return false;

    switch (type) {
    case OpenRandMirageWhenClose:
        return m_settings->value("Window/OpenRandMirageWhenClose", false).toBool();
    case UseLightTheme:
        return m_settings->value("Window/UseLightTheme", false).toBool();
    case RunAsClient:
        return m_settings->value("RPWeb/RunAsClient", false).toBool();
    case InstantPickByDefault:
        return m_settings->value("InPick/InstantPickByDefault", false).toBool();
    case TopmostByDefault:
        return m_settings->value("InPick/TopmostByDefault", false).toBool();
    }
    return false;
}

QString SettingsHandler::getStringConfig(StringConfigType type) const
{
    if (!m_settings) return QString();

    switch (type) {
    case ServerHost:
        return m_settings->value("RPWeb/Server", "localhost").toString();
    case ServerPort:
        return m_settings->value("RPWeb/Port", "8080").toString();
    }
    return QString();
}
