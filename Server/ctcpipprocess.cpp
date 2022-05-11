#include <QThreadPool>
#include <QTcpSocket>
#include <QNetworkProxy>
#include <QDataStream>

#include "ctcpipserver.h"
#include "ctcpipprocess.h"

cTcpIpProcess::cTcpIpProcess(QObject *parent)
    : QObject(parent)
    , mSocket(nullptr)
{
}

void cTcpIpProcess::initializeTcpIpSocket(qintptr handle)
{
    mSocket = new QTcpSocket(this);
    mSocket->setProxy(QNetworkProxy::NoProxy);
    cTcpIpServer::consoleWriteDebug("A new socket created!");

    connect(mSocket, &QTcpSocket::disconnected, this, &cTcpIpProcess::killProcess);
    connect(mSocket, &QTcpSocket::readyRead, this, &cTcpIpProcess::read);

    mSocket->setSocketDescriptor(handle);
    cTcpIpServer::consoleWriteDebug(QString("Client connected at %1").arg(handle));
}

bool cTcpIpProcess::isWorked()
{
    return mSocket !=nullptr;
}

void cTcpIpProcess::killProcess()
{
    cTcpIpServer::consoleWriteDebug("Client disconnected");
    mSocket->deleteLater();
    mSocket = nullptr;
}

void cTcpIpProcess::read()
{
    cTcpIpServer::consoleWriteDebug("cTcpIpProcess::read function");
    if (!mSocket){
        cTcpIpServer::consoleWriteError("socket is empty");
        return;
    }

    cTcpIpServer::consoleWriteDebug(QString("Socket %1 is ready to read").arg(mSocket->socketDescriptor()));
    buffer.clear();
    buffer = mSocket->readAll();
    cTcpIpServer::consoleWriteDebug(QString("Socket %1 readed %2").arg(mSocket->socketDescriptor()).arg(buffer.data()));
    emit socketReaded(this, buffer);
}

void cTcpIpProcess::writeToSocket(QByteArray ba)
{
    cTcpIpServer::consoleWriteDebug("cTcpIpProcess::writeToSocket function");
    if (!mSocket){
        cTcpIpServer::consoleWriteError("socket is empty");
        return;
    }
    if (mSocket->isValid()){
        cTcpIpServer::consoleWriteDebug(QString("Socket %1 write: %2").arg(mSocket->socketDescriptor()).arg(ba.data()));
        mSocket->write(ba);
    }
}
