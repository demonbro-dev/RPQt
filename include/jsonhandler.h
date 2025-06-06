// jsonhandler.h
#ifndef JSONHANDLER_H
#define JSONHANDLER_H

#include <QObject>
#include <QMap>
#include <QStringList>

class JsonHandler : public QObject
{
    Q_OBJECT
public:
    explicit JsonHandler(QObject *parent = nullptr);

    QMap<QString, QStringList> loadFromFile(const QString &filePath, QString &error);
    QString getPassphrase(const QString &filePath, QString &error);

    bool saveToFile(const QMap<QString, QStringList> &data,
                    const QString &filePath,
                    QString &error);
    bool savePassphrase(const QString &passphrase,
                        const QString &filePath,
                        QString &error);
    bool createDefaultNamelist(const QString &filePath, QString &error);

private:
    QString defaultFileName = "namelist.json";
    QByteArray b64Control(const QByteArray &data, bool encode, QString &error);
    QJsonDocument parseJsonData(const QByteArray &data, QString &error);
    bool writeJsonToFile(const QString &filePath, const QJsonDocument &doc, QString &error);
};

#endif // JSONHANDLER_H
