#include "ctcpipserver.h"
#include <QtNetwork>
#include <QNetworkProxy>
#include <QNetworkInterface>
#include <stdio.h>
#include <iostream>
#include <QString>
#include "ctcpipprocess.h"
#include "ccommandexecutor.h"
#include <QTimer>
#include <QThread>
#include <QCoreApplication>
#include "servercommands.h"
#include  <QList>

bool cTcpIpServer::mDebugInfoEnable = true;
bool cTcpIpServer::mErrorInfoEnable = true;
bool cTcpIpServer::mInfoEnable = true;

cTcpIpServer::cTcpIpServer(QObject * parent)
    : QTcpServer(parent)
    , mExecutor(nullptr)
    , udpSocket (new QUdpSocket(this))
{
    udpSocket->setProxy(QNetworkProxy::NoProxy);
}

cTcpIpServer::~cTcpIpServer()
{
    stop();
}

void cTcpIpServer::initialize()
{
    udpSocket->bind(SERVER_TCPIP_PORT + 1, QUdpSocket::ShareAddress);
    connect(udpSocket, &QUdpSocket::readyRead, [=](){
        QByteArray datagram;
        updateAvailableTcpIpAddresses();
        while (udpSocket->hasPendingDatagrams()){
            datagram.resize(int(udpSocket->pendingDatagramSize()));
            udpSocket->readDatagram(datagram.data(), datagram.size());
            consoleWriteDebug(tr("Getted UDP diagram: %1").arg(datagram.constData()));
            QStringList adList = QString(datagram).split(";");
            for(QString addr: adList){
                if (!addr.isEmpty()){
                    QTcpSocket socket;
                    socket.setProxy(QNetworkProxy::NoProxy);
                    socket.connectToHost(QHostAddress(addr), SERVER_TCPIP_PORT + 2);
                    qDebug()<<QHostAddress(addr);
                    qApp->processEvents();
                    socket.waitForConnected(100);
                    if (socket.isOpen())
                        consoleWriteDebug("Send TcpIpAddress to "+addr);
                    socket.write(availableTcpIpAddresses.toLocal8Bit());
                    socket.flush();
                }
            }
        }
    });

    consoleWriteDebug("Start command executor thread");
    if (!mExecutor){
        consoleWriteError("CommandExecutor are not exist!!!");
        return;
    }

    QThread *thread = new QThread(this);
    mExecutor->moveToThread(thread);

    connect(thread, &QThread::started, mExecutor, &cCommandExecutor::doWork);
    connect(mExecutor, &cCommandExecutor::finished, thread, &QThread::quit);
    connect(mExecutor, &cCommandExecutor::finished, mExecutor, &cCommandExecutor::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    connect(mExecutor, &cCommandExecutor::inited, this, &cTcpIpServer::startServer);
    connect(mExecutor, &cCommandExecutor::sendAnswer, this, &cTcpIpServer::sendAnswer);

    thread->start();
}

void cTcpIpServer::stop()
{
    if (!mExecutor)
        mExecutor->stop();
}

void cTcpIpServer::consoleWrite(QString string)
{
    QString ts = QTime::currentTime().toString("hh:mm:ss:zzz ");
    if (mInfoEnable)
        std::cout<< ts.toLatin1().data()<<string.toLatin1().data()<<std::endl;
}

void cTcpIpServer::consoleWriteDebug(QString string)
{
    QString ts = QTime::currentTime().toString("hh:mm:ss:zzz ");
    if (mInfoEnable && mDebugInfoEnable)
        std::cout<< ts.toLatin1().data()<<"Debug: "<<string.toLatin1().data()<<std::endl;
}

void cTcpIpServer::consoleWriteError(QString string)
{
    QString ts = QTime::currentTime().toString("hh:mm:ss:zzz ");
    if (mInfoEnable && mErrorInfoEnable)
        std::cerr << ts.toLatin1().data() << "Error: "<<string.toLatin1().data()<<std::endl;
}

//void cTcpIpServer::consoleWriteHWStats()
//{
//    //   QString ts = QTime::currentTime().toString("hh:mm:ss:zzz ");
//    //   if (mInfoEnable && mErrorInfoEnable)
//    //      std::cout << ts.toLatin1().data() << HWResources::systemLoad().toLatin1().data();
//}

void cTcpIpServer::incomingConnection(qintptr handle)
{
    cTcpIpProcess *process = new cTcpIpProcess(this);
    process->initializeTcpIpSocket(handle);

    connect(process, &cTcpIpProcess::socketReaded, mExecutor, &cCommandExecutor::executeCommand);
}

void cTcpIpServer::updateAvailableTcpIpAddresses()
{
    availableTcpIpAddresses = QString();
    for (QHostAddress address: QNetworkInterface::allAddresses())
        if (address.protocol() == QAbstractSocket::IPv4Protocol)
            availableTcpIpAddresses.append(address.toString()+";");
}

bool cTcpIpServer::isInfoEnabled() const
{
    return mInfoEnable;
}

void cTcpIpServer::setInfoEnable(bool infoEnable)
{
    mInfoEnable = infoEnable;
}

bool cTcpIpServer::isErrorInfoEnabled() const
{
    return mErrorInfoEnable;
}

void cTcpIpServer::setErrorInfoEnable(bool errorInfoEnable)
{
    mErrorInfoEnable = errorInfoEnable;
}

bool cTcpIpServer::isDebugInfoEnabled() const
{
    return mDebugInfoEnable;
}

void cTcpIpServer::setDebugInfoEnable(bool debugInfoEnable)
{
    mDebugInfoEnable = debugInfoEnable;
}

cCommandExecutor *cTcpIpServer::executor() const
{
    return mExecutor;
}

void cTcpIpServer::setExecutor(cCommandExecutor *executor)
{
    mExecutor = executor;
}

void cTcpIpServer::startServer()
{
    consoleWriteDebug("Start TcpIp Server");
    setProxy(QNetworkProxy::NoProxy);
    if (listen(QHostAddress::Any, SERVER_TCPIP_PORT)){
        consoleWrite("Server: started");
        consoleWrite("Available TcpIp addresses:");
        updateAvailableTcpIpAddresses();
        consoleWrite(availableTcpIpAddresses);
        consoleWrite(QString("Available TcpIp Port: %1").arg(QString::number(SERVER_TCPIP_PORT)));
    }
    else
        consoleWrite("Server: not started!");
}

void cTcpIpServer::sendAnswer(QObject *process, QByteArray data)
{
    consoleWriteDebug("cTcpIpServer::sendAnswer");


    if (process){
        auto tmp = qobject_cast<cTcpIpProcess*>(process);
        if (tmp){
            if (!tmp->isWorked()){
                tmp->deleteLater();
                tmp = nullptr;
            }
            else
                tmp->writeToSocket(data);
        }
    }
}
