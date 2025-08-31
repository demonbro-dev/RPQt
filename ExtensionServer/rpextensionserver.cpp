#include "rpextensionserver.h"
#include <QDebug>

RPExtensionServer::RPExtensionServer(QObject *parent)
    : QObject(parent)
    , m_server(new QLocalServer(this))
    , m_speech(new QTextToSpeech(this))
{
    connect(m_server, &QLocalServer::newConnection, this, &RPExtensionServer::handleNewConnection);
}

RPExtensionServer::~RPExtensionServer()
{
    m_server->close();
}

bool RPExtensionServer::startServer(const QString &serverName)
{
    QLocalServer::removeServer(serverName);
    if (!m_server->listen(serverName)) {
        qWarning() << "Failed to start server:" << m_server->errorString();
        return false;
    }

    qInfo() << "RPExtensionServer started on:" << serverName;
    return true;
}

void RPExtensionServer::handleNewConnection()
{
    QLocalSocket *socket = m_server->nextPendingConnection();
    if (!socket) return;

    connect(socket, &QLocalSocket::readyRead, this, &RPExtensionServer::readSocketData);
    connect(socket, &QLocalSocket::disconnected, this, &RPExtensionServer::socketDisconnected);

    m_bufferMap[socket] = QByteArray();
    qInfo() << "New client connected";
}

void RPExtensionServer::readSocketData()
{
    QLocalSocket *socket = qobject_cast<QLocalSocket*>(sender());
    if (!socket) return;

    QByteArray &buffer = m_bufferMap[socket];
    buffer.append(socket->readAll());

    // Process complete messages (assuming messages are newline separated)
    while (buffer.contains('\n')) {
        int pos = buffer.indexOf('\n');
        QByteArray message = buffer.left(pos).trimmed();
        buffer = buffer.mid(pos + 1);

        if (!message.isEmpty()) {
            processCommand(socket, message);
        }
    }
}

void RPExtensionServer::socketDisconnected()
{
    QLocalSocket *socket = qobject_cast<QLocalSocket*>(sender());
    if (socket) {
        m_bufferMap.remove(socket);
        socket->deleteLater();
        qInfo() << "Client disconnected";
    }
}

void RPExtensionServer::processCommand(QLocalSocket *socket, const QByteArray &data)
{
    QString command = QString::fromUtf8(data);

    // Simple command protocol: "COMMAND:DATA"
    if (command.startsWith("TTS:")) {
        QString text = command.mid(4);
        handleTTSCommand(text);

        // Send acknowledgment
        if (socket && socket->state() == QLocalSocket::ConnectedState) {
            socket->write("TTS_ACK\n");
            socket->flush();
        }
    }
    // Add more command handlers here for future extensions
    else {
        qWarning() << "Unknown command:" << command;
    }
}

void RPExtensionServer::handleTTSCommand(const QString &text)
{
    if (text.isEmpty()) return;

    qInfo() << "Speaking text:" << text;
    m_speech->say(text);
}
