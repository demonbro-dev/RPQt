// fbshandler.h
#ifndef FBSHANDLER_H
#define FBSHANDLER_H

#include <QObject>
#include <QMap>
#include <QStringList>
#include <QByteArray>
#include <QStandardPaths>
#include <flatbuffers/flatbuffers.h>

#ifdef Q_OS_WIN
#define NAMELIST_PATH_BINARY QCoreApplication::applicationDirPath() + "/namelist.bin"
#define PICKED_PERSISTENT_NAMELIST_PATH QCoreApplication::applicationDirPath() + "/.nlpersist"
#else
#define NAMELIST_PATH_BINARY QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/namelist.bin"
#define PICKED_PERSISTENT_NAMELIST_PATH QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/.nlpersist"
#endif

class FbsHandler : public QObject
{
    Q_OBJECT
public:
    explicit FbsHandler(QObject *parent = nullptr);

    flatbuffers::DetachedBuffer parseFbsData(const QByteArray &data, QString &error);
    bool writeFbsToFile(const QString &filePath, const flatbuffers::DetachedBuffer &buffer, QString &error);

    QString getPassphrase(const QString &filePath, QString &error);
    bool savePassphrase(const QString &passphrase, const QString &filePath, QString &error);

    QMap<QString, QStringList> loadFromFile(const QString &filePath, QString &error);
    bool saveToFile(const QMap<QString, QStringList> &data, const QString &filePath, QString &error);
    bool saveToPersistFile(const QMap<QString, QStringList> &data, QString &error);
    bool createDefaultNamelist(const QString &filePath, QString &error);
};

#endif // FBSHANDLER_H
