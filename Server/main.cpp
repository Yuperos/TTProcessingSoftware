#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>


#include "ctcpipserver.h"
#include "ccommandexecutor.h"


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCoreApplication::setApplicationName("cu-tcpipserver");


    cTcpIpServer *server = new cTcpIpServer();
    cCommandExecutor *executor = new cCommandExecutor();
//    executor->setInterface(mInterface);
    server->setExecutor(executor);
//    server->setInfoEnable(parser.isSet(commonInfoOption));
//    server->setDebugInfoEnable(parser.isSet(debugInfoOption));
//    server->setErrorInfoEnable(parser.isSet(errorOption));


    server->initialize();
    return a.exec();
}
