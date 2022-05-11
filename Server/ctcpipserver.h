#ifndef CTCPIPSERVER_H
#define CTCPIPSERVER_H

#include <QTcpServer>

class cCommandExecutor;
class QUdpSocket;

class cTcpIpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit cTcpIpServer(QObject * parent = nullptr);
    ~cTcpIpServer() override;

    void initialize();
    void stop();

    static void consoleWrite(QString string);
    static void consoleWriteDebug(QString string);
    static void consoleWriteError(QString string);
//    static void consoleWriteHWStats();

    cCommandExecutor *executor() const;
    void setExecutor(cCommandExecutor *executor);

    bool isDebugInfoEnabled() const;
    void setDebugInfoEnable(bool isDebugInfoEnabled);

    bool isErrorInfoEnabled() const;
    void setErrorInfoEnable(bool isErrorInfoEnabled);

    bool isInfoEnabled() const;
    void setInfoEnable(bool isInfoEnabled);

protected:
    void incomingConnection(qintptr handle) override;

private:
    cCommandExecutor *mExecutor;
    static bool mDebugInfoEnable;
    static bool mErrorInfoEnable;
    static bool mInfoEnable;
    QString availableTcpIpAddresses;

    QUdpSocket *udpSocket;
    void updateAvailableTcpIpAddresses();

private slots:
    void startServer();
    void sendAnswer(QObject * process, QByteArray data);
};

#endif // CTCPIPSERVER_H
