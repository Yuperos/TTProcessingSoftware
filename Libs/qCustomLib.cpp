#include "qCustomLib.h"
#include <QtNetwork>
#include "servercommands.h"

#include <QObject>
#include <QElapsedTimer>
#include <QDebug>

QHostAddress convertToHostAddress(QString str)
   {
   QStringList strL = str.split('.');
   /// Данная функция нужна, чтобы не путались восьмиричные числа
   /// в случае чего используем стандартную функцию.
   if (strL.size()!=4) return QHostAddress(str);
   quint32 result = 0;
   for (int i = 0; i <4 ; ++i){
      result = result << 8;
      bool ok;
      result += QString(strL[i]).toInt(&ok);
      if (!ok) return QHostAddress(str);
      }
   return QHostAddress(result);
   }

QStringList availableControlUnits()
   {
   // рассылаем Udp запросы
   QStringList answer;
   QTcpServer* server = new QTcpServer();

   server->setProxy(QNetworkProxy::NoProxy);
   if(!server->listen(QHostAddress::Any, SERVER_TCPIP_PORT + 2)){
      return answer;
      }

   QObject::connect(server, &QTcpServer::newConnection, [=, &server, &answer](){
      QTcpSocket* socket = server->nextPendingConnection();
      socket->setProxy(QNetworkProxy::NoProxy);
      bool ok = socket->waitForReadyRead(1000);
      if (ok){
         QByteArray ba = socket->readAll();
         answer.append(QString(ba).split(";"));
         }
      socket->deleteLater();
      });

//   qDebug()<<"before udpSocket create";
   QUdpSocket udpSocket;
   udpSocket.setProxy(QNetworkProxy::NoProxy);
   QString tcpipAddresses;
   for (QHostAddress address: QNetworkInterface::allAddresses())
      if (address.protocol() == QAbstractSocket::IPv4Protocol)
         if (address != QHostAddress::LocalHost)
            tcpipAddresses.append(address.toString()+";");

   QByteArray datagram = QByteArray(tcpipAddresses.toLocal8Bit());
   udpSocket.writeDatagram(datagram, QHostAddress::Broadcast, SERVER_TCPIP_PORT + 1);
   QElapsedTimer timer;
   timer.start();
   while (timer.elapsed()<500)
      qApp->processEvents();

//   qDebug()<<"waiting for answers";

   server->deleteLater();

   // подчищаем все лишнее
   // заменяем собственные адреса на localhost
   for (QStringList::iterator i = answer.begin(); i < answer.end();){
      if ((*i).isEmpty() || (QHostAddress(*i) == QHostAddress::LocalHost))
         i = answer.erase(i);
      else ++i;
      }

   for (QHostAddress address: QNetworkInterface::allAddresses())
      if (address.protocol() == QAbstractSocket::IPv4Protocol){
         //проверяем есть ли он в списке И меняем его на Localhost
         int i = answer.indexOf(address.toString());
         while (i>=0){
            answer[i] = "localhost";
            i = answer.indexOf(address.toString(), i+1);
            }
         }
   // удаляем повторы
   answer = answer.toSet().toList();

   int i = answer.indexOf("localhost");
   if (i>=0) answer[i] = "127.0.0.1";

   return answer;
   }

