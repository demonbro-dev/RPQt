// jsonhandler.cpp
#include "jsonhandler.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QCoreApplication>
#include <QByteArray>

JsonHandler::JsonHandler(QObject *parent) : QObject(parent)
{
}

QByteArray JsonHandler::b64Control(const QByteArray &data, bool encode, QString &error)
{
    error.clear();
    if (data.isEmpty()) {
        error = "输入数据为空";
        return QByteArray();
    }

    return encode ? data.toBase64() : QByteArray::fromBase64(data);
}

QJsonDocument JsonHandler::parseJsonData(const QByteArray &data, QString &error)
{
    error.clear();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        error = QString("JSON解析错误: %1").arg(parseError.errorString());
    }

    return doc;
}

bool JsonHandler::writeJsonToFile(const QString &filePath, const QJsonDocument &doc, QString &error)
{
    error.clear();
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        error = QString("文件保存失败: %1").arg(file.errorString());
        return false;
    }

    QByteArray encodedData = b64Control(doc.toJson(), true, error);
    if (!error.isEmpty()) return false;

    file.write(encodedData);
    file.close();
    return true;
}

QString JsonHandler::getPassphrase(const QString &filePath, QString &error)
{
    error.clear();
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        error = QString("文件打开失败: %1").arg(file.errorString());
        return QString();
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QByteArray decodedData = b64Control(jsonData, false, error);
    if (!error.isEmpty()) return QString();

    QJsonDocument doc = parseJsonData(decodedData.isEmpty() ? jsonData : decodedData, error);
    if (!error.isEmpty()) return QString();

    return doc.object()["passphrase"].toObject()["phrasevalue"].toString();
}

bool JsonHandler::savePassphrase(const QString &passphrase,
                                 const QString &filePath,
                                 QString &error)
{
    error.clear();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        error = QString("文件打开失败: %1").arg(file.errorString());
        return false;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QByteArray decodedData = b64Control(jsonData, false, error);
    if (!error.isEmpty()) return false;

    QJsonDocument doc = parseJsonData(decodedData.isEmpty() ? jsonData : decodedData, error);
    if (!error.isEmpty()) return false;

    QJsonObject rootObj = doc.object();
    QJsonObject passphraseObj;
    passphraseObj["phrasevalue"] = passphrase;
    rootObj["passphrase"] = passphraseObj;

    return writeJsonToFile(filePath, QJsonDocument(rootObj), error);
}

QMap<QString, QStringList> JsonHandler::loadFromFile(const QString &filePath, QString &error)
{
    QMap<QString, QStringList> nameGroups;
    error.clear();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        error = QString("文件打开失败: %1").arg(file.errorString());
        return nameGroups;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QByteArray decodedData = b64Control(jsonData, false, error);
    if (!error.isEmpty()) return nameGroups;

    QJsonDocument doc = parseJsonData(decodedData.isEmpty() ? jsonData : decodedData, error);
    if (!error.isEmpty()) return nameGroups;

    QJsonArray lists = doc.object()["name_lists"].toArray();
    for (const QJsonValue &listValue : lists) {
        QJsonObject listObj = listValue.toObject();
        QString listName = listObj["name"].toString();
        QJsonArray members = listObj["members"].toArray();

        QStringList memberList;
        for (const QJsonValue &memberValue : members) {
            memberList << memberValue.toString();
        }

        if (!listName.isEmpty() && !memberList.isEmpty()) {
            nameGroups.insert(listName, memberList);
        }
    }

    if (nameGroups.isEmpty()) {
        error = "文件未包含有效名单数据";
    }

    return nameGroups;
}

bool JsonHandler::saveToFile(const QMap<QString, QStringList> &data,
                             const QString &filePath,
                             QString &error)
{
    error.clear();

    QFile file(filePath);
    if (!file.exists()) {
        QJsonObject rootObj;
        rootObj["name_lists"] = QJsonArray();
        rootObj["passphrase"] = QJsonObject();
        rootObj["passphrase"].toObject()["phrasevalue"] = "";

        return writeJsonToFile(filePath, QJsonDocument(rootObj), error);
    }

    if (!file.open(QIODevice::ReadOnly)) {
        error = QString("文件打开失败: %1").arg(file.errorString());
        return false;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QByteArray decodedData = b64Control(jsonData, false, error);
    if (!error.isEmpty()) return false;

    QJsonDocument doc = parseJsonData(decodedData.isEmpty() ? jsonData : decodedData, error);
    if (!error.isEmpty()) return false;

    QJsonObject rootObj = doc.object();
    QJsonObject passphraseObj = rootObj["passphrase"].toObject();
    QString existingPassphrase = passphraseObj["phrasevalue"].toString();

    QJsonArray listsArray;
    for (auto it = data.cbegin(); it != data.cend(); ++it) {
        QJsonObject listObj;
        listObj["name"] = it.key();

        QJsonArray membersArray;
        for (const QString &member : it.value()) {
            membersArray.append(member);
        }
        listObj["members"] = membersArray;

        listsArray.append(listObj);
    }

    rootObj["name_lists"] = listsArray;
    passphraseObj["phrasevalue"] = existingPassphrase;
    rootObj["passphrase"] = passphraseObj;

    return writeJsonToFile(filePath, QJsonDocument(rootObj), error);
}

bool JsonHandler::createDefaultNamelist(const QString &filePath, QString &error)
{
    error.clear();

    QJsonObject rootObj;
    QJsonArray nameListsArray;

    // 创建默认名单组
    QJsonObject defaultGroup;
    defaultGroup["name"] = "Default1";
    QJsonArray membersArray;
    membersArray.append("12");
    membersArray.append("34");
    membersArray.append("56");
    defaultGroup["members"] = membersArray;
    nameListsArray.append(defaultGroup);

    // 创建空密码对象
    QJsonObject passphraseObj;
    passphraseObj["phrasevalue"] = "";

    rootObj["name_lists"] = nameListsArray;
    rootObj["passphrase"] = passphraseObj;

    return writeJsonToFile(filePath, QJsonDocument(rootObj), error);
}
