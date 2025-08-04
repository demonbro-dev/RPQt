#include "rpweb.h"
#include "ui_rpweb.h"
#include <QMessageBox>
#include <QNetworkInterface>
#include <QTimer>

RPWeb::RPWeb(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RPWeb),
    webSocketServer(nullptr),
    fbsHandler(new FbsHandler(this)),
    pickerLogic(new PickerLogic(this)),
    m_useE2EE(false)
{
    ui->setupUi(this);
    setWindowTitle(tr("RandPicker WebSocket Panel"));
    ui->portSpinBox->setRange(1024, 65535);
    ui->portSpinBox->setValue(8080);

    connect(ui->useE2EE, &QCheckBox::checkStateChanged,
            this, [this](Qt::CheckState state) {
                m_useE2EE = (state == Qt::Checked);
            });
    connect(ui->confirmButton, &QPushButton::clicked,
            this, &RPWeb::onConfirmButtonClick);

    setupCommandHandlers();
}

RPWeb::~RPWeb()
{
    if (webSocketServer) {
        webSocketServer->close();
        qDeleteAll(m_connectedClients);
        delete webSocketServer;
    }
    delete ui;
}

void RPWeb::setupCommandHandlers()
{
    m_commandHandlers = {
        {"GET_RANDOM", &RPWeb::processRandomRequest},
        {"LIST_GROUPS", &RPWeb::processListRequest},
        {"E2EE_STATUS", &RPWeb::processE2EEStatus}
    };
}

void RPWeb::onConfirmButtonClick()
{
    if (!serverRunning) {
        int port = ui->portSpinBox->value();
        webSocketServer = new QWebSocketServer("RandPickerServer", QWebSocketServer::NonSecureMode, this);

        connect(webSocketServer, &QWebSocketServer::newConnection,
                this, [this]() {
                    QWebSocket *clientSocket = webSocketServer->nextPendingConnection();
                    handleWebSocketConnection(clientSocket);
                });

        if (!webSocketServer->listen(QHostAddress::Any, port)) {
            QMessageBox::critical(this, tr("Error"),
                                  tr("Cannot start server on port %1:%2")
                                      .arg(port)
                                      .arg(webSocketServer->errorString()));
            delete webSocketServer;
            webSocketServer = nullptr;
            return;
        }

        serverRunning = true;
        ui->confirmButton->setText(tr("Stop Server"));
        ui->portSpinBox->setEnabled(false);

        QString ipAddress;
        QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
        for (const QHostAddress &address : ipAddressesList) {
            if (address != QHostAddress::LocalHost && address.toIPv4Address()) {
                ipAddress = address.toString();
                break;
            }
        }
        ui->statusLabel->setText(
            tr("Server Running\nAddress: ws://%1:%2")
                .arg(ipAddress.isEmpty() ? "127.0.0.1" : ipAddress)
                .arg(port));
    } else {
        webSocketServer->close();
        qDeleteAll(m_connectedClients);
        m_connectedClients.clear();
        delete webSocketServer;
        webSocketServer = nullptr;

        serverRunning = false;
        ui->confirmButton->setText(tr("Start Server"));
        ui->portSpinBox->setEnabled(true);
        ui->statusLabel->setText(tr("Server is stopped"));
    }
}

void RPWeb::handleWebSocketConnection(QWebSocket *clientSocket)
{
    m_connectedClients.insert(clientSocket);

    connect(clientSocket, &QWebSocket::textMessageReceived,
            this, [this, clientSocket](const QString &message) {
                processCommand(clientSocket, message);
            });

    connect(clientSocket, &QWebSocket::disconnected, this, [this, clientSocket]() {
        m_connectedClients.remove(clientSocket);
        clientSocket->deleteLater();
    });
}

void RPWeb::processCommand(QWebSocket *clientSocket, const QString &message)
{
    QStringList parts = message.split(' ', Qt::SkipEmptyParts);
    if (parts.isEmpty()) {
        sendResponse(clientSocket, "Invalid Command", true);
        return;
    }

    QString command = parts[0];
    QString argument = parts.size() > 1 ? parts.mid(1).join(' ') : QString();

    if (m_commandHandlers.contains(command)) {
        (this->*m_commandHandlers[command])(clientSocket, argument);
    } else {
        QStringList validCommands = m_commandHandlers.keys();
        sendResponse(clientSocket,
                     QString("Invalid Command. Available Commands:%1").arg(validCommands.join(", ")),
                     true);
    }
}

void RPWeb::processRandomRequest(QWebSocket *clientSocket, const QString &argument)
{
    QStringList args = argument.split(' ', Qt::SkipEmptyParts);
    if (args.isEmpty()) {
        sendResponse(clientSocket, "Usage: GET_RANDOM [LIST_NAME] [COUNT]", true);
        return;
    }

    QString listName = args[0];
    int count = 1;
    if (args.size() > 1) {
        bool ok;
        count = args[1].toInt(&ok);
        if (!ok || count <= 0) {
            sendResponse(clientSocket, "Invalid count parameter. Must be a positive integer", true);
            return;
        }
    }

    QString error;
    QMap<QString, QStringList> nameGroups = fbsHandler.loadFromFile(NAMELIST_PATH_BINARY, error);

    if (!error.isEmpty()) {
        sendResponse(clientSocket, QString("Error when loading namelist: %1").arg(error), true);
    } else if (!nameGroups.contains(listName)) {
        sendResponse(clientSocket, QString("List '%1' doesn't exist").arg(listName), true);
    } else {
        pickerLogic->setNames(nameGroups[listName]);
        // If count is >= total names, pick all names
        if (count >= nameGroups[listName].size()) {
            count = nameGroups[listName].size();
        }
        QStringList pickedNames = pickerLogic->pickNames(count, true, PickerLogic::RandomGeneratorType::RandomSelect);
        sendResponse(clientSocket, pickerLogic->formatNamesWithLineBreak(pickedNames));
    }
}

void RPWeb::processListRequest(QWebSocket *clientSocket, const QString &)
{
    QString error;
    QMap<QString, QStringList> nameGroups = fbsHandler.loadFromFile(NAMELIST_PATH_BINARY, error);

    if (!error.isEmpty()) {
        sendResponse(clientSocket, QString("Error when loading namelist: %1").arg(error), true);
    } else {
        sendResponse(clientSocket,
                     QString("Available Lists: %1").arg(nameGroups.keys().join(", ")));
    }
}

void RPWeb::processE2EEStatus(QWebSocket *clientSocket, const QString &)
{
    sendResponse(clientSocket, m_useE2EE ? "true" : "false");
}

void RPWeb::sendResponse(QWebSocket *clientSocket, const QString &message, bool isError)
{
    QString prefix = isError ? "[ERROR] " : "";
    clientSocket->sendTextMessage(prefix + message);
}
