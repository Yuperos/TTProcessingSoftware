#ifndef CCOMMANDEXECUTER_H
#define CCOMMANDEXECUTER_H

#include <QObject>
#include <QTimer>
#include <QMap>
#include "CmdTypes.h"


class cuIOInterface;
class cAbstractCommandParser;

class cCommandExecutor : public QObject
{
    Q_OBJECT
public:
    explicit cCommandExecutor(QObject *parent = nullptr);

    void prepareAnswer(quint8 address, quint8 command, quint8 dataLength, char *data);
    void prepareAnswer(QString answer);
    bool addDevice(quint8 address);

//    cuIOInterface *inter face() const;
//    void setInterface(cuIOInterface *interface);

    void moveToThread(QThread *thread);

signals:
    void finished();
    void inited();
    void sendAnswer(QObject * process, QByteArray data);

public slots:
    void doWork();
    void stop();
    void executeCommand(QObject* tcpIpProcess, QByteArray data);

private slots:
    void process();

private:
//    cuIOInterface *mInterface;
    QTimer *processTimer;
    QList<QPair <QObject*, QByteArray> > cmdList;
    QObject* currentTcpIpProcess;
    QMap<CmdType, cAbstractCommandParser*> parsers;
    bool mStopFlag;

    void initialize();

};

#endif // CCOMMANDEXECUTER_H
