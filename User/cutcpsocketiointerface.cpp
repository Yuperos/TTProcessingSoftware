#include "cutcpsocketiointerface.h"
#include <QElapsedTimer>
#include <QCoreApplication>
#include <QNetworkProxy>
#include <QThread>
#include <QDataStream>
#include "servercommands.h"

cuTcpSocketIOInterface::cuTcpSocketIOInterface(QObject *parent)
   : cuIOInterfaceImpl(parent)
   , mSocket(new QTcpSocket(this))
   , mAddress(QHostAddress::Null)
   , mPort(SERVER_TCPIP_PORT)
   {
   buffer.clear();
   mSocket->setProxy(QNetworkProxy::NoProxy);
   connect(mSocket, SIGNAL(readyRead()), this, SLOT(dataReady()));
   }

cuTcpSocketIOInterface::~cuTcpSocketIOInterface()
   {
   if (!mSocket->isOpen()){
      mSocket->close();
      mSocket->abort();
      }
   mSocket->deleteLater();
   }

QHostAddress cuTcpSocketIOInterface::address() const
   {
   return mAddress;
   }

void cuTcpSocketIOInterface::setAddress(const QHostAddress &address)
   {
   mAddress = address;
   if (mSocket->isOpen()) mSocket->close();
   }

quint16 cuTcpSocketIOInterface::port() const
   {
   return mPort;
   }

void cuTcpSocketIOInterface::setPort(const quint16 &port)
   {
   mPort = port;
   if (mSocket->isOpen()) mSocket->close();
   }

QString cuTcpSocketIOInterface::tcpIpQuery(QString query, int TimeOut, bool *ok)
   {
   if (!isSocketReady()) {
      initialize();
      if (!isSocketReady()){
         if (ok)
            *ok = false;
         return QString();
         }
      }

   buffer.clear();
   mSocket->readAll();
   isReadyRead = false;

   disconnect(mSocket, SIGNAL(readyRead()), this, SLOT(dataReady()));

   mSocket->write(query.toLatin1());

   QElapsedTimer timer;
   timer.start();
   while (timer.elapsed()<TimeOut && !isReadyRead){
      qApp->processEvents();
      };

   connect(mSocket, SIGNAL(readyRead()), this, SLOT(dataReady()), Qt::QueuedConnection);

   buffer.append(mSocket->readAll());
   qDebug() << "buffer len" << buffer.length();
   if (ok)
      *ok = true;
   return QString(buffer);
   }

bool cuTcpSocketIOInterface::pSendMsg(quint8 address, quint8 command, quint8 dataLength, quint8 *data)
   {
   if (!isSocketReady()) {
      initialize();
      if (!isSocketReady())
         return false;
      }

   QByteArray ba;
   ba.append(address);
   ba.append(command);
   ba.append(dataLength);
   ba.append((char*) data, dataLength);
   mSocket->write(ba);
   return true;
   }

bool cuTcpSocketIOInterface::pInitialize()
   {
   if (!isSocketReady()){
      mSocket->connectToHost(address(), port());
      }
   mSocket->waitForConnected(1000);
   return mSocket->state() == QAbstractSocket::ConnectedState;
   }

void cuTcpSocketIOInterface::dataReady()
   {

      buffer.append(mSocket->readAll());
      if (buffer.size() < 3) return;
      if (buffer.size() == buffer[2] + 3) {
         isReadyRead = true;
         emit msgReceived(buffer[0],
               buffer[1],
               buffer[2],
               ((quint8*) buffer.data()) + 3);
         }
   }

bool cuTcpSocketIOInterface::isSocketReady()
   {
   return (mSocket->isOpen() && (mSocket->state() & QAbstractSocket::ConnectedState));
   }
