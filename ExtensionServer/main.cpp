#include "rpextensionserver.h"
#include <QDebug>
#include <QCoreApplication>
#include <QLocalSocket>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    RPExtensionServer server;
    if (!server.startServer()) {
        return 1;
    }

    return a.exec();
}
