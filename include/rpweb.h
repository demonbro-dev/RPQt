#ifndef RPWEB_H
#define RPWEB_H

#include <QDialog>
#include <QHttpServer>
#include <QTcpServer>

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
    QHttpServer *httpServer;
    bool serverRunning = false;
};

#endif // RPWEB_H
