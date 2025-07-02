// main.cpp
#include "mainwindow.h"
#include "settingshandler.h"
#include <QApplication>
#include <QFile>
#include <QStyleFactory>
#include <QPalette>
#include <QTranslator>
#include <QLibraryInfo>
#include <QLocale>
#include <QCommandLineParser>
#include <QMessageBox>
#include <QLocalSocket>
#include <QLocalServer>

bool isAlreadyRunning(const QString &uniqueKey)
{
    QLocalSocket socket;
    socket.connectToServer(uniqueKey);
    if (socket.waitForConnected(105)) {
        socket.close();
        return true;
    }

    QLocalServer::removeServer(uniqueKey);
    QLocalServer *server = new QLocalServer();
    if (!server->listen(uniqueKey)) {
        qWarning() << "Failed to create single-instance server:" << server->errorString();
    }
    return false;
}

int main(int argc, char *argv[])
{
    {
        QApplication tmpApp(argc, argv);
        const QString uniqueKey = "RandPicker_UniqueKey_" +
                                  QCoreApplication::applicationName();

        if (isAlreadyRunning(uniqueKey)) {
            QMessageBox::critical(nullptr, "Error", "A RandPicker instance is already running.", QMessageBox::Ok);
            return 1;
        }
    }

    QApplication a(argc, argv);

    QCommandLineParser parser;
    QCommandLineOption sidebarOption("sidebar", "Start application in sidebar mode");
    QCommandLineOption sidebarLeftOption("sidebar-left", "Start application in sidebar mode (left side)");
    parser.addOption(sidebarOption);
    parser.addOption(sidebarLeftOption);
    parser.process(a);

    SettingsHandler settingsHandler;
    a.setStyle(QStyleFactory::create("Fusion"));

    if (settingsHandler.getBoolConfig(SettingsHandler::UseLightTheme)) {
        QPalette lightPalette;
        lightPalette.setColor(QPalette::Window, Qt::white);
        lightPalette.setColor(QPalette::WindowText, Qt::black);
        lightPalette.setColor(QPalette::Base, QColor(240,240,240));
        lightPalette.setColor(QPalette::AlternateBase, Qt::white);
        lightPalette.setColor(QPalette::ToolTipBase, Qt::white);
        lightPalette.setColor(QPalette::ToolTipText, Qt::black);
        lightPalette.setColor(QPalette::Text, Qt::black);
        lightPalette.setColor(QPalette::Button, QColor(240,240,240));
        lightPalette.setColor(QPalette::ButtonText, Qt::black);
        lightPalette.setColor(QPalette::BrightText, Qt::red);
        lightPalette.setColor(QPalette::Link, QColor(38,191,102));
        lightPalette.setColor(QPalette::Highlight, QColor(227,227,227));
        lightPalette.setColor(QPalette::HighlightedText, Qt::black);
        a.setPalette(lightPalette);
        QFile style(":/data/styleLight.qss");
        style.open(QFile::ReadOnly);
        a.setStyleSheet(style.readAll());
        style.close();
    } else {
        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window, QColor(31,31,31));
        darkPalette.setColor(QPalette::WindowText, Qt::white);
        darkPalette.setColor(QPalette::Base, QColor(46,47,48));
        darkPalette.setColor(QPalette::AlternateBase, QColor(53,53,53));
        darkPalette.setColor(QPalette::ToolTipBase, QColor(46,47,48));
        darkPalette.setColor(QPalette::ToolTipText, Qt::white);
        darkPalette.setColor(QPalette::Text, Qt::white);
        darkPalette.setColor(QPalette::Button, QColor(33,33,33));
        darkPalette.setColor(QPalette::ButtonText, Qt::white);
        darkPalette.setColor(QPalette::BrightText, Qt::red);
        darkPalette.setColor(QPalette::Link, QColor(38,191,102));
        darkPalette.setColor(QPalette::Highlight, QColor(112,128,144));
        darkPalette.setColor(QPalette::HighlightedText, Qt::white);
        a.setPalette(darkPalette);
        QFile style(":/data/styleDark.qss");
        style.open(QFile::ReadOnly);
        a.setStyleSheet(style.readAll());
        style.close();
    }

    MainWindow w;

    // 检查是否带有 --sidebar 参数
    if (parser.isSet(sidebarOption) || parser.isSet(sidebarLeftOption)) {
        bool toRight = parser.isSet(sidebarOption);
        QTimer::singleShot(0, [&w, toRight]() {
            w.showSideButton(toRight);
        });
    } else {
        w.show();
    }

    return a.exec();
}
