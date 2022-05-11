#include "generalscpicommands.h"
#include "ctcpipserver.h"

GeneralScpiCommands::GeneralScpiCommands(QObject * parent)
    : CommonScpiCommands(parent)
{

}

/** формат строки
 * GENeral:DEVice<N>:INIT - инициализация устройства
 * GENeral:DEVice<N>:PARKing - парковка устройства перед его отключением
 * GENeral:DEVice<N>:ListeningOFF - устройство перестает откликаться на команды, далее работает после перезагрузки
 * GENeral:DEVice<N>:STATus? - получение статуса устройства
 * GENeral:DEVice<N>:ERRor? - получение последней ошибки
 * GENeral:DEVice<N>:DeviceID? - получение уникального идентификатора устройства
 * GENeral:DEVice<N>:DeviceTYPe? - получение типа устройства
 * GENeral:DEVice<N>:DeviceDEScription? - получение полного описания устройства
 * GENeral:DEVice<N>:MODificationVersion? - получение номера модификации устройства
 * GENeral:DEVice<N>:HardWareVersion? - получение версии железа
 * GENeral:DEVice<N>:FirmWareVersion? - получение версии программного обеспечения
 * GENeral:DEVice<N>:reBOOT - перезагрузка устройства
 * GENeral:DEVice<N>:WriteEEProm - запись основных констант в память
 */

bool GeneralScpiCommands::executeCommand(CmdToken token)
{
    if (!executor()){
        cTcpIpServer::consoleWriteError("executor empty");
        return false;
    }
    return false;
}
