#include "rpweb.h"
#include "ui_rpweb.h"
#include <QMessageBox>
#include <QNetworkInterface>
#include <QFile>

RPWeb::RPWeb(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RPWeb),
    httpServer(nullptr)
{
    ui->setupUi(this);
    setWindowTitle(tr("RPWeb Server Control"));

    // 设置端口范围
    ui->portSpinBox->setRange(1, 65535);
    ui->portSpinBox->setValue(8080);
}

RPWeb::~RPWeb()
{
    if (httpServer) {
        delete httpServer;
    }
    delete ui;
}

void RPWeb::on_confirmButton_clicked()
{
    if (!serverRunning) {
        // 启动服务器
        int port = ui->portSpinBox->value();

        httpServer = new QHttpServer(this);

        // 加载HTML文件内容
        QFile htmlFile(":/web/index.html"); // 假设文件已添加到Qt资源系统
        QString htmlContent;
        if (htmlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            htmlContent = htmlFile.readAll();
            htmlFile.close();
        } else {
            QMessageBox::critical(this, tr("Error"), tr("Failed to load HTML file"));
            return;
        }

        // 绑定路由，提供HTML内容
        httpServer->route("/", [htmlContent]() {
            return htmlContent.toUtf8();
        });

        // 使用QTcpServer进行监听
        QTcpServer *tcpServer = new QTcpServer(this);
        if (!tcpServer->listen(QHostAddress::Any, port)) {
            QMessageBox::critical(this, tr("Error"),
                                  tr("Cannot set up server on port %1:%2")
                                      .arg(port)
                                      .arg(tcpServer->errorString()));
            delete tcpServer;
            return;
        }

        if (!httpServer->bind(tcpServer)) {
            QMessageBox::critical(this, tr("Error"),
                                  tr("Cannot bind HTTP Server to TCP Server"));
            tcpServer->close();
            delete tcpServer;
            return;
        }

        serverRunning = true;
        ui->confirmButton->setText(tr("Stop Server"));
        ui->portSpinBox->setEnabled(false);

        // 显示服务器信息
        QString ipAddress;
        QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
        for (const QHostAddress &address : ipAddressesList) {
            if (address != QHostAddress::LocalHost && address.toIPv4Address()) {
                ipAddress = address.toString();
                break;
            }
        }
        if (ipAddress.isEmpty()) {
            ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
        }

        ui->statusLabel->setText(
            tr("Server is running\nAddress: http://%1:%2")
                .arg(ipAddress)
                .arg(port));
    } else {
        // 停止服务器
        httpServer->deleteLater();
        httpServer = nullptr;

        serverRunning = false;
        ui->confirmButton->setText(tr("Start Server"));
        ui->portSpinBox->setEnabled(true);
        ui->statusLabel->setText(tr("Server is stopped"));
    }
}
