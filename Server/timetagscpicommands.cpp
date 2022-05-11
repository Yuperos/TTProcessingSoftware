#include "timetagscpicommands.h"
#include "ctcpipserver.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDateTime>
#include "ccommandexecutor.h"
#include "swabiancontrol.h"

TimeTagScpiCommands::TimeTagScpiCommands(QObject *parent)
   : CommonScpiCommands(parent)
   {

   }

bool TimeTagScpiCommands::executeCommand(CmdToken token)
   {
   if (!executor()){
      cTcpIpServer::consoleWriteError("executor empty");
      return false;
      }
   else
      {
      processingAnswer answer = PA_None;

      answer = processingCommand(token.mDevAddress, QString(token.mCommand), QString(token.mParams));

      switch (answer) {
         case CommonScpiCommands::PA_Ok:
            executor()->prepareAnswer("OK\r\n");
            break;
         case CommonScpiCommands::PA_TimeOut:
            executor()->prepareAnswer("ERROR: Timeout\r\n");
            break;
         case CommonScpiCommands::PA_ErrorData:
            executor()->prepareAnswer("ERROR: Wrong parameters\r\n");
            break;
         case CommonScpiCommands::PA_None:
            return false;
         case CommonScpiCommands::PA_WithoutAnswer:
            return true;
         }
      return true;
      }
   }

CommonScpiCommands::processingAnswer TimeTagScpiCommands::processingCommand(quint8 address, QString command, QString params)
   {
   SwabianControl* driver = SwabianControl::getInstance();

   bool ok = false;

   cTcpIpServer::consoleWrite("processing TimeTag Command");

   if (command.indexOf("CON") == 0){
      QString license;
      int channelCount = driver->connectTimeTagger();
      license = driver->getLicenseInfo();
      executor()->prepareAnswer(QString("%1\n%2\r\n").arg(QString::number(channelCount)).arg(license));
      return PA_WithoutAnswer;
      }

   if (command.indexOf("SRC") == 0){
      ok = driver->configSourceChannel(TTOpExchangeStruct::fromString(params));
      executor()->prepareAnswer(QString("%1\r\n").arg(static_cast<bool>(ok)));
      return PA_WithoutAnswer;
      }

   if (command.indexOf("SYNC") == 0){
      cTcpIpServer::consoleWriteDebug(params);
      int channel = driver->syncOperation(TTOpExchangeStruct::fromString(params));
      executor()->prepareAnswer(QString("%1\r\n").arg(QString::number(channel)));
      return PA_WithoutAnswer;
      }

   if (command.indexOf("RAW") == 0){
      cTcpIpServer::consoleWriteDebug(params);
      QList<int> tempVector;
      QStringList channels = params.split(",",Qt::SkipEmptyParts);
      ok = false;
      if (!channels.isEmpty()){
         for(auto a : channels){
            tempVector.append(a.toInt());
            }
         ok = driver->setupSaveRaw(tempVector);
         }
      executor()->prepareAnswer(QString("%1,%2\r\n")
                                .arg(static_cast<bool>(ok))
                                .arg(QString::number(channels.count())));
      return PA_WithoutAnswer;
      }

   if (command.indexOf("CRT?") == 0){
      QString output;
      QList<double> values = driver->getCountrateValues();
      cTcpIpServer::consoleWriteDebug(QString::number(values.count()));
      if (values.count() > 0){
//         qDebug() << values;
//         qDebug() << QDateTime(QDate(2022,1,1),QTime(0,0,0)) << QDa   teTime::currentDateTime() << QDateTime(QDate(2022,0,0),QTime(0,0,0)).msecsTo(QDateTime::currentDateTime());
         QString globalTimeStamp = QString::number(QDateTime(QDate(2022,1,1),QTime(0,0,0)).msecsTo(QDateTime::currentDateTime()));
         output.append(QString("%1,").arg(globalTimeStamp));
         if (driver->getCountrateChannelsList().count() == values.count()){
            for(auto idx : driver->getCountrateChannelsList())
               output.append(QString("%1|").arg(QString::number(idx)));
            output.chop(1);
            output.append(QString(","));
            for(auto a : values)
               output.append(QString("%1|").arg(QString::number(a)));
            output.chop(1);
            executor()->prepareAnswer(output);
            return PA_WithoutAnswer;
            }
         }
      }
   else if (command.indexOf("CRT") == 0){
      QList<int> tempVector;
      QStringList channels = params.split(",",Qt::SkipEmptyParts);
      ok = false;
      if (!channels.isEmpty()){
         for(auto a : channels){
            tempVector.append(a.toInt());
            }
         ok = driver->setupCountrate(tempVector);
         }
      executor()->prepareAnswer(QString("%1,%2\r\n")
                                .arg(static_cast<bool>(ok))
                                .arg(QString::number(channels.count())));

      return PA_WithoutAnswer;
      }
   return PA_None;
   }
