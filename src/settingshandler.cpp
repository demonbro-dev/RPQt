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

QVector<SettingsHandler::ConfigItem> SettingsHandler::getConfigItems()
{
    return {
        //{section, key, friendlyName, defaultValue, type, options, enumValue},
        {"Window", "OpenRandMirageWhenClose", tr("Open RandMirage When Closing Program"), false, "bool", {}, OpenRandMirageWhenClose},
        {"Window", "UseLightTheme", tr("Use Light Theme"), false, "bool", {}, UseLightTheme},
        {"Window", "Language", tr("Language"), "Default", "list", {"Default", "en_US", "zh_CN"}, Language},
        {"InPick", "InstantPickByDefault", tr("Enable Instant Pick by default"), false, "bool", {}, InstantPickByDefault},
        {"InPick", "TopmostByDefault", tr("Enable Topmost by default"), false, "bool", {}, TopmostByDefault},
        {"RPWeb", "RunAsClient", tr("Run as Client"), false, "bool", {}, RunAsClient},
        {"RPWeb", "Server", tr("Server Host"), "localhost", "string", {}, ServerHost},
        {"RPWeb", "Port", tr("Server Port"), "8080", "string", {}, ServerPort}
    };
}

bool SettingsHandler::getBoolConfig(BoolConfigType type) const
{
    if (!m_settings) return false;

    QVector<ConfigItem> items = getConfigItems();
    for (const auto &item : items) {
        if (item.type == "bool" && item.enumValue.isValid() &&
            item.enumValue.toInt() == type) {
            return getConfigValue(item).toBool();
        }
    }

    return false;
}

QString SettingsHandler::getStringConfig(StringConfigType type) const
{
    if (!m_settings) return QString();

    QVector<ConfigItem> items = getConfigItems();
    for (const auto &item : items) {
        if ((item.type == "string" || item.type == "list") && item.enumValue.isValid() &&
            item.enumValue.toInt() == type) {
            return getConfigValue(item).toString();
        }
    }

    return QString();
}

QString SettingsHandler::getListConfig(ListConfigType type) const
{
    if (!m_settings) return QString();

    QVector<ConfigItem> items = getConfigItems();
    for (const auto &item : items) {
        if (item.type == "list" && item.enumValue.isValid() &&
            item.enumValue.toInt() == type) {

            QString value = getConfigValue(item).toString();

            if (!item.options.contains(value)) {
                return item.defaultValue.toString();
            }

            return value;
        }
    }

    return QString();
}

QVariant SettingsHandler::getConfigValue(const QString &section, const QString &key) const
{
    if (!m_settings) return QVariant();
    return m_settings->value(section + "/" + key);
}

QVariant SettingsHandler::getConfigValue(const ConfigItem &item) const
{
    if (!m_settings) return item.defaultValue;
    return m_settings->value(item.section + "/" + item.key, item.defaultValue);
}
