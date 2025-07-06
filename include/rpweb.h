#ifndef RPWEB_H
#define RPWEB_H

#include <QDialog>
#include <QWebSocketServer>
#include <QTcpSocket>
#include <QHash>
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

private slots:
    void on_confirmButton_clicked();

private:
    Ui::RPWeb *ui;
    QWebSocketServer *webSocketServer;
    JsonHandler *jsonHandler;
    PickerLogic *pickerLogic;
    bool serverRunning = false;
    QSet<QWebSocket *> m_connectedClients;
};

#endif // RPWEB_H
