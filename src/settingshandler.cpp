#include "settingshandler.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>

SettingsHandler::SettingsHandler(QObject *parent) : QObject(parent), m_openRandMirageWhenClose(false)
{
#ifdef Q_OS_WIN
    QString configPath = QCoreApplication::applicationDirPath() + "/config.ini";
#else
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/config.ini";
#endif

    if (QFile::exists(configPath)) {
        m_settings = new QSettings(configPath, QSettings::IniFormat, this);

        m_openRandMirageWhenClose = m_settings->value("Window/OpenRandMirageWhenClose", false).toBool();
    } else {
        m_settings = nullptr;
    }
}

SettingsHandler::~SettingsHandler()
{
    delete m_settings;
}

bool SettingsHandler::generateExampleConfig()
{
#ifdef Q_OS_WIN
    QString configPath = QCoreApplication::applicationDirPath() + "/config.ini";
#else
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/config.ini";
#endif
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
    out << "# OpenRandMirageWhenClose=false\n";

    file.close();
    return true;
}

bool SettingsHandler::getOpenRandMirageWhenClose() const
{
    return m_openRandMirageWhenClose;
}
