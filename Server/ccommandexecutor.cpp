#include "ccommandexecutor.h"
#include <QThread>
#include <QCoreApplication>
#include <QSettings>
#include "ctcpipserver.h"
#include "servercommands.h"
#include "commandparser.h"
#include "cuiointerface.h"

cCommandExecutor::cCommandExecutor(QObject *parent)
   : QObject(parent)
   //    , mInterface(nullptr)
   , processTimer(nullptr)
   , mStopFlag(false)
   {
   parsers.insert(CT_System,       new SystemScpiCommands(this));
   parsers.insert(CT_General,      new GeneralScpiCommands(this));
   parsers.insert(CT_TimeTagger,   new TimeTagScpiCommands(this));
   }

void cCommandExecutor::doWork()
   {
   cTcpIpServer::consoleWriteDebug("Command executor started");
   initialize();

   processTimer = new QTimer(this);
   processTimer->setInterval(10);
   processTimer->setSingleShot(true);
   connect(processTimer, &QTimer::timeout, this, &cCommandExecutor::process);
   process();
   }

void cCommandExecutor::stop()
   {
   cTcpIpServer::consoleWriteDebug("Command executor get STOP signal");
   mStopFlag = true;
   emit finished();
   }

void cCommandExecutor::executeCommand(QObject *tcpIpProcess, QByteArray data)
   {
   cTcpIpServer::consoleWrite(QString("New command execute"));
   if (data.indexOf('\n') > data.count())
      data.remove(data.indexOf('\n'),-1);
   cmdList.append(qMakePair(tcpIpProcess, data));
//   if (data.contains(QByteArray("\r\n")))
//      for(int i = 0; i < cmdList.count(); i++){
//         QList<QByteArray> tempArray = cmdList.at(i).second.split('\n');
//         for(int j = 0; j < tempArray.count(); j++){
//            tempArray[i].append('\n');
//            cmdList.insert(i+j,qMakePair(cmdList.at(i).first,tempArray.at(i)));
//            i++;
//            }
//         }
//   if (cmdList.count() > 1)
//      cmdList.erase(cmdList.begin()+2);
   }

void cCommandExecutor::process()
   {
   // выполняем команду, после чего пускаем все заново
   if (!cmdList.isEmpty()){
      // пришла команда
      cTcpIpServer::consoleWriteDebug(QString("CmdList buffer size: %1. Start to execute command.").arg(cmdList.count()));

      // проверяем есть ли еще в стеке команды, если есть, то удаляем и переходим у следующей команде
      bool sameProcessFounded = false;
      for (auto iter = cmdList.begin() + 1; iter < cmdList.end(); ++iter){
         if (iter->first == cmdList.begin()->first){
            sameProcessFounded = true;
            break;
            }
         }

      if (!sameProcessFounded){
         CmdToken cmdToken;
         currentTcpIpProcess = cmdList.begin()->first;
         for (auto parser : parsers.values())
            cmdToken = parser->parse(cmdList.begin()->second);

         if (!cmdToken.isEmpty())
            parsers.value(cmdToken.mCmdType)->executeCommand(cmdToken);
         else
            prepareAnswer("UNKNOWN COMMAND\r\n");
         }

      // в конце удаляем команду из начала списка команд
      cmdList.removeAt(0);
      }
   processTimer->start();
   }

void cCommandExecutor::moveToThread(QThread *thread)
   {
   //    mInterface->moveToThread(thread);
       QObject::moveToThread(thread);
   }

//cuIOInterface *cCommandExecutor::interface() const
//{
//    return mInterface;
//}

//void cCommandExecutor::setInterface(cuIOInterface *interface)
//{
//    mInterface = interface;
//}

void cCommandExecutor::initialize()
   {
   cTcpIpServer::consoleWriteDebug("Initialize Comand Executor");

   for (auto parser : parsers.values())
      parser->initializeParser();

   emit inited();
   }

void cCommandExecutor::prepareAnswer(quint8 address, quint8 command, quint8 dataLength, char *data)
   {
   cTcpIpServer::consoleWriteDebug("Preparing answer");

   QByteArray ba;
   ba.append(*reinterpret_cast<char*>(&address));
   ba.append(*reinterpret_cast<char*>(&command));
   ba.append(*reinterpret_cast<char*>(&dataLength));
   ba.append(data, dataLength);

   cTcpIpServer::consoleWriteDebug(QString("Answer: %1").arg(ba.toHex().data()));

   emit sendAnswer(currentTcpIpProcess, ba);
   //    emit sendAnswer(cmdList.begin()->first, ba);
   }

void cCommandExecutor::prepareAnswer(QString answer)
   {
//   cTcpIpServer::consoleWriteDebug("Preparing answer as string");
//   cTcpIpServer::consoleWriteDebug(QString("Answer: %1").arg(answer));

   //    emit sendAnswer(cmdList.begin()->first, QByteArray(answer.toLocal8Bit()));
   emit sendAnswer(currentTcpIpProcess, QByteArray(answer.toLocal8Bit()));
   }

bool cCommandExecutor::addDevice(quint8 address)
   {
   //    cTcpIpServer::consoleWriteDebug(QString("Add device with address %1").arg(address));
   //    mSettings->removeDeviceWithAddress(address);

   //    CommonDriver driver;
   //    driver.setIOInterface(mInterface);
   //    driver.setDevAddress(address);
   //    cDeviceInfo info;
   //    bool ok = false;
   //    info.setAddress(address);
   //    info.setType(driver.deviceType()->getValueSync(&ok, 5));
   //    if (!ok) return false;

   //    info.setUDID(driver.UDID()->getValueSync(&ok,5));
   //    if (!ok) return false;

   //    info.setModificationVersion(driver.modificationVersion()->getValueSync(&ok, 5));
   //    if (!ok) return false;

   //    info.setHardwareVersion(driver.hardwareVersion()->getValueSync(&ok, 5));
   //    if (!ok) return false;

   //    info.setFirmwareVersion(driver.firmwareVersion()->getValueSync(&ok, 5));
   //    if (!ok) return false;

   //    info.setDescription(driver.deviceDescription()->getValueSync(&ok, 5));
   //    if (!ok) return false;

   //    mSettings->append(info);
   cTcpIpServer::consoleWriteDebug(QString("SUCCESS"));
   return true;
   }
