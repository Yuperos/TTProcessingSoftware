#ifndef CUTCPSOCKETIOINTERFACE_H
#define CUTCPSOCKETIOINTERFACE_H

#include <QObject>
#include "cuiointerfaceimpl.h"
#include <QTcpSocket>
#include <QHostAddress>

class cuTcpSocketIOInterface : public cuIOInterfaceImpl
   {
   Q_OBJECT
public:
   QTcpSocket *mSocket;
   bool isReadyRead = false;

   explicit cuTcpSocketIOInterface(QObject *parent = nullptr);
   ~cuTcpSocketIOInterface();

   QHostAddress address() const;
   void setAddress(const QHostAddress &address);

   quint16 port() const;
   void setPort(const quint16 &port);

   QString tcpIpQuery(QString query, int TimeOut, bool *ok = nullptr);
   bool isSocketReady();

protected:
   bool pSendMsg(quint8 address, quint8 command, quint8 dataLength, quint8 *data);
   bool pInitialize();

private slots:
   void dataReady();

signals:
   void error(int socketError, const QString &message);
   void newPackage(QString buffer);

private:
   QHostAddress mAddress;
   QByteArray buffer;
   quint16 mPort;

   };

#endif // CUTCPSOCKETIOINTERFACE_H
