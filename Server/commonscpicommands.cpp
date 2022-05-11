#include "commonscpicommands.h"
#include "ctcpipserver.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

CommonScpiCommands::CommonScpiCommands(QObject *parent)
   : cAbstractCommandParser (parent)
   {

   }

bool CommonScpiCommands::isCorrectCommand(const QByteArray &ba)
   {
   Q_UNUSED(ba)
   return true;
   }

CmdType CommonScpiCommands::extractCmdType(QString str)
   {
   CmdType rv = _CT_NA;
   if (str.indexOf("TAG") == 0) rv = CT_TimeTagger;
   else if (str.indexOf("GEN")  == 0) rv = CT_General;
   else if (str.indexOf("SYST") == 0) rv = CT_System;

   if (rv == _CT_NA)
      cTcpIpServer::consoleWriteError(QString("Bad \"Request type\" SCPI Field: %1").arg(str));
   return rv;
   }

uint8_t CommonScpiCommands::extractDevNumber(QString str)
   {
   uint8_t rv = 0xff;
   if (str.contains("DEV")){
      str.remove(0,3);
      bool ok = false;
      uint8_t temp = str.toInt(&ok);
      if (ok)
         rv = temp;
      else if (str.contains('L'))
         rv = 0;
      }
   else
      cTcpIpServer::consoleWriteError(QString("Bad DEV<N> SCPI Field: %1").arg(str));
   return rv;
   }

CmdToken CommonScpiCommands::parseCommand(const QByteArray &ba)
   {
   CmdToken rv;
   // преобразуем в QString;
   QString tmpStr = QString(ba).simplified();
   QStringList tempList = tmpStr.split(" ");
   QStringList cmdList = tempList[0].split(":");
   while(cmdList.length() < 4)
      cmdList.append("");
   rv.mCmdType = extractCmdType(cmdList[0]);
   if (rv.mCmdType == CT_Common){
      rv.mCommand.append(cmdList[0]);
      return rv;
      }
   //   rv.mDevAddress = extractDevNumber(cmdList[1]);
   //   if (cmdList[1].contains("DEVL")){
   //      rv.mCommand.append(cmdList[1]);
   //      if (cmdList[1].at(4) != '?')
   //         rv.mCommand.append(':');
   //      }
   rv.mCommand.append(cmdList[1]);

   QString params;
   if (tempList.count() > 1){
      tempList.removeFirst();
      params = tempList.join(" ");
      }
   rv.mParams.append(params);

   return rv;
   }

bool CommonScpiCommands::executeCommand(CmdToken token)
   {
   cTcpIpServer::consoleWriteDebug(QString("SCPI command: %1. Params: %2").arg(QString(token.mCommand)).arg(QString(token.mParams)));

   if (!executor()){
      cTcpIpServer::consoleWriteError("executor empty");
      return false;
      }


   if (token.mCommand == "*IDN?"){
      executor()->prepareAnswer("Scontel ControlUnit,00001,0.01.02\r\n");
      return true;
      }

   return false;
   }

QPair<float,float> CommonScpiCommands::pairFromJsonString(QString str, bool *ok)
   {
   QPair<float,float> data;
   if (ok)
      *ok = false;
   QJsonDocument jsonDoc(QJsonDocument::fromJson(str.toUtf8()));
   if (!jsonDoc.isArray()){
      return data;
      }
   QJsonArray value = jsonDoc.array();
   if (value.size()!=2){
      return data;
      }
   data.first  = static_cast<float>(value[0].toDouble());
   data.second = static_cast<float>(value[1].toDouble());
   if (ok)
      *ok = true;
   return data;
   }

void CommonScpiCommands::sendPairFloat(QPair<float, float> data)
   {
   QJsonArray value;
   value.append(static_cast<double>(data.first));
   value.append(static_cast<double>(data.second));
   QJsonDocument jsonDoc(value);
   executor()->prepareAnswer(QString("%1\r\n").arg(QString(jsonDoc.toJson())));
   }
