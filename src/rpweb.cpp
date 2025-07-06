#include "rpweb.h"
#include "ui_rpweb.h"
#include <QMessageBox>
#include <QNetworkInterface>
#include <QTimer>

RPWeb::RPWeb(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RPWeb),
    webSocketServer(nullptr),
    jsonHandler(new JsonHandler(this)),
    pickerLogic(new PickerLogic(this))
{
    ui->setupUi(this);
    setWindowTitle(tr("RandPicker WebSocket Panel"));
    ui->portSpinBox->setRange(1024, 65535);
    ui->portSpinBox->setValue(8080);

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
        {"LIST_GROUPS", &RPWeb::processListRequest}
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

    QTimer::singleShot(60000, clientSocket, [clientSocket]() {
        if (clientSocket->state() == QAbstractSocket::ConnectedState) {
            clientSocket->sendTextMessage("Connection Timeout");
            clientSocket->close();
        }
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
    QString argument = parts.size() > 1 ? parts[1] : QString();

    if (m_commandHandlers.contains(command)) {
        (this->*m_commandHandlers[command])(clientSocket, argument);
    } else {
        QStringList validCommands = m_commandHandlers.keys();
        sendResponse(clientSocket,
                     QString("Invalid Command. Available Commands:%1").arg(validCommands.join(", ")),
                     true);
    }
}

void RPWeb::processRandomRequest(QWebSocket *clientSocket, const QString &listName)
{
    if (listName.isEmpty()) {
        sendResponse(clientSocket, "Usage: GET_RANDOM <LIST_NAME>", true);
        return;
    }

    QString error;
    QMap<QString, QStringList> nameGroups = jsonHandler->loadFromFile(NAMELIST_PATH, error);

    if (!error.isEmpty()) {
        sendResponse(clientSocket, QString("Error when loading namelist: %1").arg(error), true);
    } else if (!nameGroups.contains(listName)) {
        sendResponse(clientSocket, QString("List '%1' doesn't exist").arg(listName), true);
    } else {
        pickerLogic->setNames(nameGroups[listName]);
        QStringList pickedNames = pickerLogic->pickNames(10, true, PickerLogic::RandomGeneratorType::RandomSelect);
        sendResponse(clientSocket, pickerLogic->formatNamesWithLineBreak(pickedNames));
    }
}

void RPWeb::processListRequest(QWebSocket *clientSocket, const QString &)
{
    QString error;
    QMap<QString, QStringList> nameGroups = jsonHandler->loadFromFile(NAMELIST_PATH, error);

    if (!error.isEmpty()) {
        sendResponse(clientSocket, QString("Error when loading namelist: %1").arg(error), true);
    } else {
        sendResponse(clientSocket,
                     QString("Available Lists: %1").arg(nameGroups.keys().join(", ")));
    }
}

void RPWeb::sendResponse(QWebSocket *clientSocket, const QString &message, bool isError)
{
    QString prefix = isError ? "[ERROR] " : "";
    clientSocket->sendTextMessage(prefix + message);
}
