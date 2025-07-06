#ifndef RPWEB_H
#define RPWEB_H

#include <QDialog>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QSet>
#include "jsonhandler.h"
#include "pickerlogic.h"

namespace Ui {
class RPWeb;
}

class RPWeb : public QDialog
{
    Q_OBJECT

public:
    explicit RPWeb(QWidget *parent = nullptr);
    ~RPWeb();


private:
    void onConfirmButtonClick();
    void handleWebSocketConnection(QWebSocket *clientSocket);
    void processCommand(QWebSocket *clientSocket, const QString &message);
    void processRandomRequest(QWebSocket *clientSocket, const QString &listName);
    void processListRequest(QWebSocket *clientSocket);
    void sendResponse(QWebSocket *clientSocket, const QString &message, bool isError = false);
    void processListRequest(QWebSocket *clientSocket, const QString &);

    using CommandHandler = void (RPWeb::*)(QWebSocket*, const QString&);
    void setupCommandHandlers();

    Ui::RPWeb *ui;
    QWebSocketServer *webSocketServer;
    QSet<QWebSocket *> m_connectedClients;
    JsonHandler *jsonHandler;
    PickerLogic *pickerLogic;
    bool serverRunning = false;

    QHash<QString, CommandHandler> m_commandHandlers;
};

#endif // RPWEB_H
