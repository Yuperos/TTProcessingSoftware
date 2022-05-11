#include "systemscpicommands.h"
#include "ctcpipserver.h"
//#include "cuiodevice.h"

SystemScpiCommands::SystemScpiCommands(QObject *parent)
    : CommonScpiCommands(parent)
{

}

/** формат строки
 * SYSTem:DEViceList? - device list
 * SYSTem:DEViceList:ADD <N> - добавить новый, N - адрес
 * SYSTem:DEViceList:FIND - поиск новых устройств. Очень долгая команда
 * SYSTem:DEViceList:SAVE - сохранить список устройств
 */
bool SystemScpiCommands::executeCommand(CmdToken token)
{
    if (!executor()){
        cTcpIpServer::consoleWriteError("executor empty");
        return false;
    }

    QStringList strList = QString(token.mCommand).split(':');

    if (strList[0] != "DEVL")
        return false;

    strList.removeFirst();      // удаляем DEVL
    if (strList.count() == 0)   //команда неправильная
        return false;

    if (strList[0] == "ADD"){
        bool ok;
        quint8 address = static_cast<quint8>(QString(token.mParams).toInt(&ok));
        if (!ok) return false;
        ok = executor()->addDevice(address);
        executor()->prepareAnswer(ok ? "OK\r\n" : QString("ERROR AT ADDING DEVICE %1\r\n").arg(address));
        return true;
    }

    return false;
}
