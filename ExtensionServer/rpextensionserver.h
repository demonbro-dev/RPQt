#ifndef RPEXTENSIONSERVER_H
#define RPEXTENSIONSERVER_H

#include <QObject>
#include <QLocalServer>
#include <QLocalSocket>
#include <QTextToSpeech>
#include <QMap>

class RPExtensionServer : public QObject
{
    Q_OBJECT
public:
    explicit RPExtensionServer(QObject *parent = nullptr);
    ~RPExtensionServer();

    bool startServer(const QString &serverName = "RPExtensionServer");

public slots:
    void handleNewConnection();
    void readSocketData();
    void socketDisconnected();

private:
    QLocalServer *m_server;
    QTextToSpeech *m_speech;
    QMap<QLocalSocket*, QByteArray> m_bufferMap;

    void processCommand(QLocalSocket *socket, const QByteArray &data);
    void handleTTSCommand(const QString &text);
};

#endif // RPEXTENSIONSERVER_H
