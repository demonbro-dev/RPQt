// fbshandler.cpp
#include "fbshandler.h"
#include <QFile>
#include <QCoreApplication>
#include <QByteArray>
#include <QDir>
#include "namelist_generated.h"

#ifdef Q_OS_WIN
#include "windows.h"
#endif

FbsHandler::FbsHandler(QObject *parent) : QObject(parent)
{
}

static bool useBase64 = true;  //该变量决定是否使用Base64编码文件

flatbuffers::DetachedBuffer FbsHandler::parseFbsData(const QByteArray &data, QString &error)
{
    error.clear();

    QByteArray actualData = data;
    if (useBase64) {
        actualData = QByteArray::fromBase64(data);
    }

    flatbuffers::Verifier verifier(reinterpret_cast<const uint8_t*>(actualData.constData()), actualData.size());
    if (!RPNameListConf::VerifyConfigBuffer(verifier)) {
        error = "FlatBuffers verification failed: Invalid format";
        return flatbuffers::DetachedBuffer();
    }

    flatbuffers::FlatBufferBuilder fbb;
    fbb.PushFlatBuffer(reinterpret_cast<const uint8_t*>(actualData.constData()), actualData.size());
    return fbb.Release();
}

bool FbsHandler::writeFbsToFile(const QString &filePath, const flatbuffers::DetachedBuffer &buffer, QString &error)
{
    error.clear();
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        error = QString("Failed to save file: %1").arg(file.errorString());
        return false;
    }

    QByteArray data(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    if (useBase64) {
        data = data.toBase64();
    }

    file.write(data);
    file.close();

#ifdef Q_OS_WIN
    std::wstring filePathW = QDir::toNativeSeparators(filePath).toStdWString();
    DWORD attributes = GetFileAttributesW(filePathW.c_str());
    if (attributes != INVALID_FILE_ATTRIBUTES) {
        SetFileAttributesW(filePathW.c_str(), attributes | FILE_ATTRIBUTE_HIDDEN);
    }
#endif

    return true;
}

QString FbsHandler::getPassphrase(const QString &filePath, QString &error)
{
    error.clear();
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        error = QString("Failed to open file: %1").arg(file.errorString());
        return QString();
    }

    QByteArray fbsData = file.readAll();
    file.close();

    auto buffer = parseFbsData(fbsData, error);
    if (!error.isEmpty()) return QString();

    auto config = RPNameListConf::GetConfig(buffer.data());
    if (config->passphrase() && config->passphrase()->phrasevalue()) {
        return QString::fromUtf8(config->passphrase()->phrasevalue()->c_str());
    }

    return QString();
}

bool FbsHandler::savePassphrase(const QString &passphrase,
                                const QString &filePath,
                                QString &error)
{
    error.clear();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        error = QString("Failed to open file: %1").arg(file.errorString());
        return false;
    }

    QByteArray fbsData = file.readAll();
    file.close();

    auto buffer = parseFbsData(fbsData, error);
    if (!error.isEmpty()) return false;

    flatbuffers::FlatBufferBuilder fbb;

    auto existingConfig = RPNameListConf::GetConfig(buffer.data());

    std::vector<flatbuffers::Offset<RPNameListConf::NameList>> nameLists;
    if (existingConfig->name_lists()) {
        for (auto it = existingConfig->name_lists()->begin(); it != existingConfig->name_lists()->end(); ++it) {
            auto name = fbb.CreateString(it->name()->str());

            std::vector<flatbuffers::Offset<flatbuffers::String>> members;
            if (it->members()) {
                for (auto mit = it->members()->begin(); mit != it->members()->end(); ++mit) {
                    members.push_back(fbb.CreateString(mit->str()));
                }
            }

            auto membersVector = fbb.CreateVector(members);
            nameLists.push_back(RPNameListConf::CreateNameList(fbb, name, membersVector));
        }
    }

    auto nameListsVector = fbb.CreateVector(nameLists);
    auto phraseValue = fbb.CreateString(passphrase.toUtf8().constData());
    auto passphraseObj = RPNameListConf::CreatePassphrase(fbb, phraseValue);

    auto config = RPNameListConf::CreateConfig(fbb, nameListsVector, passphraseObj);
    fbb.Finish(config);

    return writeFbsToFile(filePath, fbb.Release(), error);
}

QMap<QString, QStringList> FbsHandler::loadFromFile(const QString &filePath, QString &error)
{
    QMap<QString, QStringList> nameGroups;
    error.clear();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        error = QString("Failed to open file: %1").arg(file.errorString());
        return nameGroups;
    }

    QByteArray fbsData = file.readAll();
    file.close();

    auto buffer = parseFbsData(fbsData, error);
    if (!error.isEmpty()) return nameGroups;

    auto config = RPNameListConf::GetConfig(buffer.data());
    if (!config->name_lists()) {
        error = "No valid namelist data exists in the file";
        return nameGroups;
    }

    for (auto it = config->name_lists()->begin(); it != config->name_lists()->end(); ++it) {
        QString listName = QString::fromUtf8(it->name()->c_str());
        QStringList memberList;

        if (it->members()) {
            for (auto mit = it->members()->begin(); mit != it->members()->end(); ++mit) {
                memberList << QString::fromUtf8(mit->c_str());
            }
        }

        if (!listName.isEmpty() && !memberList.isEmpty()) {
            nameGroups.insert(listName, memberList);
        }
    }

    if (nameGroups.isEmpty()) {
        error = "No valid namelist data exists in the file";
    }

    return nameGroups;
}

bool FbsHandler::saveToFile(const QMap<QString, QStringList> &data,
                            const QString &filePath,
                            QString &error)
{
    error.clear();

    QFile file(filePath);
    if (!file.exists()) {
        return createDefaultNamelist(filePath, error);
    }

    if (!file.open(QIODevice::ReadOnly)) {
        error = QString("Failed to open file: %1").arg(file.errorString());
        return false;
    }

    QByteArray fbsData = file.readAll();
    file.close();

    auto buffer = parseFbsData(fbsData, error);
    if (!error.isEmpty()) return false;

    //以更新的数据重建FlatBuffers
    flatbuffers::FlatBufferBuilder fbb;

    auto existingConfig = RPNameListConf::GetConfig(buffer.data());

    QString existingPassphrase;
    if (existingConfig->passphrase() && existingConfig->passphrase()->phrasevalue()) {
        existingPassphrase = QString::fromUtf8(existingConfig->passphrase()->phrasevalue()->c_str());
    }

    std::vector<flatbuffers::Offset<RPNameListConf::NameList>> nameLists;
    for (auto it = data.cbegin(); it != data.cend(); ++it) {
        auto name = fbb.CreateString(it.key().toUtf8().constData());

        std::vector<flatbuffers::Offset<flatbuffers::String>> members;
        for (const QString &member : it.value()) {
            members.push_back(fbb.CreateString(member.toUtf8().constData()));
        }

        auto membersVector = fbb.CreateVector(members);
        nameLists.push_back(RPNameListConf::CreateNameList(fbb, name, membersVector));
    }

    auto nameListsVector = fbb.CreateVector(nameLists);
    auto phraseValue = fbb.CreateString(existingPassphrase.toUtf8().constData());
    auto passphraseObj = RPNameListConf::CreatePassphrase(fbb, phraseValue);

    auto config = RPNameListConf::CreateConfig(fbb, nameListsVector, passphraseObj);
    fbb.Finish(config);

    return writeFbsToFile(filePath, fbb.Release(), error);
}

bool FbsHandler::saveToPersistFile(const QMap<QString, QStringList> &data, QString &error)
{
    error.clear();

    // 使用主文件路径创建FlatBuffer，然后保存到持久化文件
    flatbuffers::FlatBufferBuilder fbb;

    std::vector<flatbuffers::Offset<RPNameListConf::NameList>> nameLists;
    for (auto it = data.cbegin(); it != data.cend(); ++it) {
        auto name = fbb.CreateString(it.key().toUtf8().constData());

        std::vector<flatbuffers::Offset<flatbuffers::String>> members;
        for (const QString &member : it.value()) {
            members.push_back(fbb.CreateString(member.toUtf8().constData()));
        }

        auto membersVector = fbb.CreateVector(members);
        nameLists.push_back(RPNameListConf::CreateNameList(fbb, name, membersVector));
    }

    auto nameListsVector = fbb.CreateVector(nameLists);

    // 尝试从主文件读取已有的密码短语
    QString existingPassphrase;
    QString mainError;
    existingPassphrase = getPassphrase(NAMELIST_PATH_BINARY, mainError);

    flatbuffers::Offset<RPNameListConf::Passphrase> passphraseObj;
    if (!existingPassphrase.isEmpty()) {
        auto phraseValue = fbb.CreateString(existingPassphrase.toUtf8().constData());
        passphraseObj = RPNameListConf::CreatePassphrase(fbb, phraseValue);
    } else {
        passphraseObj = RPNameListConf::CreatePassphrase(fbb);
    }

    auto config = RPNameListConf::CreateConfig(fbb, nameListsVector, passphraseObj);
    fbb.Finish(config);

    // 直接保存到持久化文件
    return writeFbsToFile(PICKED_PERSISTENT_NAMELIST_PATH, fbb.Release(), error);
}

bool FbsHandler::createDefaultNamelist(const QString &filePath, QString &error)
{
    error.clear();

    flatbuffers::FlatBufferBuilder fbb;

    //创建默认名单
    auto defaultName = fbb.CreateString("Default1");
    std::vector<flatbuffers::Offset<flatbuffers::String>> defaultMembers;
    defaultMembers.push_back(fbb.CreateString("12"));
    defaultMembers.push_back(fbb.CreateString("34"));
    defaultMembers.push_back(fbb.CreateString("56"));
    auto membersVector = fbb.CreateVector(defaultMembers);
    auto defaultList = RPNameListConf::CreateNameList(fbb, defaultName, membersVector);

    std::vector<flatbuffers::Offset<RPNameListConf::NameList>> nameLists;
    nameLists.push_back(defaultList);
    auto nameListsVector = fbb.CreateVector(nameLists);

    auto passphrase = RPNameListConf::CreatePassphrase(fbb);

    //创建配置
    auto config = RPNameListConf::CreateConfig(fbb, nameListsVector, passphrase);
    fbb.Finish(config);

    return writeFbsToFile(filePath, fbb.Release(), error);
}
