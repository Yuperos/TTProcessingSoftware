#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "channelblock.h"
#include "portlabel.h"
#include "servercommands.h"
#include "ScenePlot.h"

#include <QSettings>
#include <QMessageBox>
#include <QThread>
#include <QDebug>
#include <QTime>
#include <QRandomGenerator>

ServerCommunicationState MainWindow::getCommState() const
   {
   return commState;
   }

void MainWindow::setCommState(const ServerCommunicationState &value)
   {
   commState = value;
   }

MainWindow::MainWindow(QWidget *parent) :
   QMainWindow(parent),
   ui(new Ui::MainWindow)
   {
   ui->setupUi(this);

   scene = new TTGraphScene(this);
   plot = new CountratePlot(&storage,this);

   ui->graphicsView->setScene(scene);
   ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   QRectF bounds = scene->itemsBoundingRect();
   bounds.setWidth(bounds.width()*0.9);
   bounds.setHeight(bounds.height()*0.9);
   ui->graphicsView->fitInView(bounds, Qt::KeepAspectRatio);
   ui->graphicsView->setAutoFillBackground(true);

   tcpInterface = new cuTcpSocketIOInterface();

   connectionWatchdog = new QTimer(this);
   connectionWatchdog->setInterval(procedureInterval);
   connectionWatchdog->setSingleShot(true);
   updateStatus();
   QGraphicsProxyWidget *tempProxy = new QGraphicsProxyWidget(nullptr,Qt::Sheet);
   ScenePlot *scenePlot = new ScenePlot();
   tempProxy->setWidget(scenePlot);
   tempProxy->resize(tempProxy->minimumSize());
   scene->addItem(tempProxy);

   load();

   connect(this->connectionWatchdog,&QTimer::timeout,this,&MainWindow::comunicationProcedure);

   scene->setupLines();

   connect(ui->TB_Plot,&QToolButton::toggled,plot,&CountratePlot::setVisible);
   connect(ui->TB_DumpFile,&QToolButton::toggled,this,&MainWindow::recieveDumpClick);
   connect(plot,&CountratePlot::visibilityChanged, ui->TB_Plot, &QToolButton::setChecked);

   GraphicsViewZoom *zoom = new GraphicsViewZoom(ui->graphicsView);
   zoom->setModifiers(Qt::NoModifier);

   connectionWatchdog->start();
   }

MainWindow::~MainWindow()
   {
   save();

   delete tcpInterface;
   delete ui;
   }

void MainWindow::initOperationBlocks()
   {
   getTTOperations();
   for(auto a : ttOperations.keys()){
      addBlock(ttOperations.value(a));
      }
   }

void MainWindow::getTTOperations()
   {
   for (int i = 1; i<13; i++){
      int outChannel = i;
      TTOpExchangeWrapper *tmp = new TTOpExchangeWrapper(outChannel,TTOperationType(0),{},{});
      if (i>=6){
         for(int j = 0; j<(qrand()%6+1); j++)
            tmp->getInputChannels().append(qrand()%12+1);
         tmp->setInputChannels(tmp->getInputChannels().toSet().toList());
         }
      ttOperations.insert(outChannel,tmp);
      }
   }

void MainWindow::layoutBlocks()
   {
   }

void MainWindow::processMenuAction(QAction *act)
   {
   TTOpExchangeWrapper *newStruct = new TTOpExchangeWrapper();
   newStruct->setTtop(TTOpExchangeWrapper::stringToOpType(act->text()));
   addBlock(newStruct);
   }

void MainWindow::addBlock(TTOpExchangeWrapper *ttStruct, QRectF rect)
   {
   BlockProxyWidget *tempProxy = new BlockProxyWidget(nullptr,Qt::Sheet);

   ttStruct->check();

   if (ttStruct->getOutChannel() == 0){
      if (blocks.isEmpty() || (!blocks.isEmpty() && blocks.uniqueKeys().first() > -20) )
         ttStruct->setOutChannel(-512);
      else
         ttStruct->setOutChannel(blocks.uniqueKeys().first()-1);
      }

   if (ttStruct->getInputChannels().isEmpty()){
      if (ttStruct->getTtop() == TTOP_GatedChannel){
         for (int i = 0; i < 3; i++) {
            ttStruct->addLink(ttStruct->getOutChannel() - 512 - i);
            }
         }
      else if (ttStruct->getTtop() == TTOP_Coincendence){
         for (int i = 0; i < 2; i++) {
            ttStruct->addLink(ttStruct->getOutChannel() - 512 - i);
            }
         }
      else if (ttStruct->getTtop() != TTOP_SourceChannel)
         ttStruct->addLink(ttStruct->getOutChannel() - 512);
      }

   ttStruct->validate();

   ChannelBlock *block = new ChannelBlock(ttStruct);
   connect(block,&ChannelBlock::portClicked,this,&MainWindow::recievePortClick);
   connect(block,&ChannelBlock::blockClicked,this,&MainWindow::recieveBlockClick);
   connect(ttStruct,&TTOpExchangeWrapper::syncStageChanged,this,&MainWindow::recieveSyncStatusChanged);
   connect(ttStruct, &TTOpExchangeWrapper::countrateNeededChanged, this, &MainWindow::recieveIsCountrateNeededChanged);
   connect(ui->TB_DumpFile,&QToolButton::toggled,block,&ChannelBlock::dumpDisable);
   connect(tempProxy,&BlockProxyWidget::closePressed,this,[=](){
      scene->removeItem(tempProxy);
      removeBlock(ttStruct->getOutChannel());
      });

   blocks.insert(ttStruct->getOutChannel(),block);
   ttOperations.insert(ttStruct->getOutChannel(),ttStruct);

   tempProxy->setWidget(block);
   if (rect.isValid())
      tempProxy->setGeometry(rect);
   tempProxy->resize(tempProxy->minimumSize());
   scene->addItem(tempProxy);
   }

void MainWindow::removeBlock(int id)
   {
   qDebug() << "removeBlock" << id;
   blocks.value(id)->deleteLater();
   blocks.remove(id);
   ttOperations.value(id)->deleteLater();
   ttOperations.remove(id);
   scene->setupLines();
   }

void MainWindow::checkConnections()
   {
   if (!isServerConnected)
      isServerConnected = connectToServer();
   else
      if (isServerConnected && !isTaggerConnected)
         isTaggerConnected = connectTagger();
      else
         connectionWatchdog->setInterval(10000);

   updateStatus();
   }

void MainWindow::comunicationProcedure()
   {
   static ServerCommunicationState prevSCS = getCommState();

   findBlocksToSync();
   QTime start = QTime::currentTime();

   if (!blocksToSync.isEmpty() &&
       (getCommState() == SCS_Idle || getCommState() > SCS_TaggerConnection))
      setCommState(SCS_SyncBlocks);

   if (needToUpdateDumpChannels && getCommState() > SCS_SyncBlocks)
      if (!channelsToDump.isEmpty() && ui->TB_DumpFile->isChecked()){
         prevSCS = getCommState();
         setCommState(SCS_SyncDumpChannels);
         }

   switch(getCommState()){
      case SCS_ServerConnection:{
         if (!isServerConnected)
            isServerConnected = connectToServer();
         if(isServerConnected){
            procedureInterval = 500;
            setCommState(SCS_TaggerConnection);
            }
         else
            procedureInterval = 5000;

         updateStatus();
         } break;
      case SCS_TaggerConnection:{
         if (!isTaggerConnected)
            isTaggerConnected = connectTagger();
         if (isTaggerConnected){
            procedureInterval = 500;
            setCommState(SCS_SyncBlocks);
            }
         else
            procedureInterval = 5000;
         updateStatus();
         } break;
      case SCS_SyncBlocks:{
         procedureInterval = 500;
         if (!blocksToSync.isEmpty()){
            TTOpExchangeWrapper* tempWrapper = blocksToSync.takeFirst();
            syncBlock(tempWrapper);
            }
         else
            setCommState(SCS_SyncCountRateParams);
         } break;
      case SCS_SyncDumpChannels:{
         if (syncDumpChannels() == getDumpChannelsCount()){
            needToUpdateDumpChannels = false;
            setCommState(prevSCS);
            }
         } break;
      case SCS_SyncCountRateParams:{
         int syncedCrtOnServer = syncCountRateParams();
         int syncedCrtOnClient = getCountrateChCount();
         if (syncedCrtOnClient == syncedCrtOnServer)
            if (getCommState() != SCS_Idle)
               setCommState(SCS_GetCountrate);
         } break;
      case SCS_GetCountrate:{
         int temp = 0;
         QMap<int, double> countrate = getCountrate(temp);
         if (!countrate.isEmpty()){
            QMap<TTOpExchangeWrapper*,double> currList;
            for(auto a : countrate.uniqueKeys())
               if (ttOperations.contains(a)){
                  ttOperations.value(a)->setCountrate(countrate.value(a));
                  currList.insert(ttOperations.value(a),countrate.value(a));
                  }
            }
         } break;
      case SCS_Idle:{
         } break;
      }
   int timeLeft = procedureInterval - QTime::currentTime().msecsTo(start);
   if (timeLeft<1)
      timeLeft = 1;
   connectionWatchdog->start(timeLeft);
   }

void MainWindow::serverDisconnected()
   {
   setCommState(SCS_ServerConnection);
   isServerConnected = false;
   isTaggerConnected = false;

   for(auto a : ttOperations.uniqueKeys())
      ttOperations.value(a)->setIsConfirmed(false);
   updateStatus();
   }


bool MainWindow::connectToServer()
   {
   bool rv = false;
   QStringList serverList;
   serverList.clear();
   serverList.append(availableControlUnits());

   if (!serverList.isEmpty() && !isServerConnected){
      tcpInterface->setAddress(convertToHostAddress(serverList.first()));
      tcpInterface->setPort(SERVER_TCPIP_PORT);
      connect(tcpInterface->mSocket, &QTcpSocket::disconnected, this, &MainWindow::serverDisconnected);
      rv = true;
      }
   return rv;
   }

bool MainWindow::connectTagger()
   {
   bool rv = false;
   QString answer = tcpInterface->tcpIpQuery("TAG:CON\r\n", 6000, &rv);
   qDebug() << answer;
   int channels = answer.split("\n").at(0).toInt();
   qDebug() << "channels:" << channels;
   if (channels > 0){
      TTOpExchangeStruct::initAvaliableSourceChannels(channels);
      rv &= true;
      }
   else
      rv = false;
   return rv;
   }

bool MainWindow::syncBlock(TTOpExchangeWrapper *wrapper)
   {
   bool rv = false;
   wrapper->setSyncStage(SS_Transfer);
   QString msg = QString("TAG:SYNC %1\r\n").arg(*wrapper);
   QString answer = tcpInterface->tcpIpQuery(msg, 500, &rv);

   int prevChannel = wrapper->getOutChannel();
   int newChannel = answer.toInt(&rv);
   if (newChannel !=0 && rv){
      if (newChannel != prevChannel)
         moveBlock(prevChannel,newChannel);
      ttOperations.value(newChannel)->setSyncStage(SS_Done);
      ttOperations.value(newChannel)->setIsConfirmed(true);
      }
   else
      wrapper->setSyncStage(SS_Ready);
   return rv;
   }

void MainWindow::moveBlock(int prevIdx, int newIdx)
   {
   checkAndResolveMoveConflicts(prevIdx,newIdx);
   auto ttTemp = ttOperations.take(prevIdx);
   ttOperations.insert(newIdx,ttTemp);
   auto blockTemp = blocks.take(prevIdx);
   blocks.insert(newIdx,blockTemp);
   for (auto a : ttOperations.uniqueKeys()){
      ttOperations.value(a)->replaceLinkByNumber(prevIdx,newIdx);
      }
   //      ttOperations.value(newIdx)->replaceLink(prevIdx,newIdx);
   blocks.value(newIdx)->updateInputs();
   scene->setLinksReconfigured(true);
   }

void MainWindow::checkAndResolveMoveConflicts(int prevIdx, int newIdx)
   {
   if (ttOperations.contains(newIdx)){//INFO: надо оттестировать и решить вопрос со снятием флага "подтверждён"
      ttOperations.value(newIdx)->setIsConfirmed(false);
      if (ttOperations.uniqueKeys().first() < -1023)
         moveBlock(newIdx,blocks.uniqueKeys().first()-1);
      else
         moveBlock(newIdx,-1024);
      }
   }


int MainWindow::syncCountRateParams()
   {
   int rv = 0;
   QString channelList;
   for(auto a : ttOperations)
      if (a->getSyncStage() == SS_Done && a->getIsCountrateNeeded())
         channelList.append(QString("%1,").arg(QString::number(a->getOutChannel())));
   if (channelList.count() > 0)
      channelList.chop(1);
   else{
      channelList.append("0");
      setCommState(SCS_Idle);
      }
   if (channelList.length()>0){
      QString msg = QString("TAG:CRT %1\r\n").arg(channelList);
      qDebug() << msg;
      QStringList answer = tcpInterface->tcpIpQuery(msg, 500, (bool*)&rv).split(',');
      qDebug() << answer;
      if (answer.count() == 2){
         bool isOk = false;
         int answeredChCount = answer.at(1).toInt(&isOk);
         if (answer.at(0).indexOf('1') == 0 && isOk){
            rv = answeredChCount;
            qDebug() << "channelCount answer:" << answeredChCount;
            }
         else
            ui->TB_DumpFile->setChecked(false);
         }
      }
   return rv;
   }

int MainWindow::syncDumpChannels()
   {
   int rv = 0;
   QString channelList;
   for(auto a : channelsToDump)
      channelList.append(QString("%1,").arg(QString::number(a)));
   if (channelList.count() > 0)
      channelList.chop(1);
   else{
      channelList.append("0");
      setCommState(SCS_Idle);
      }
   if (channelList.length()>0){
      QString msg = QString("TAG:RAW %1\r\n").arg(channelList);
      qDebug() << msg;
      QStringList answer = tcpInterface->tcpIpQuery(msg, 1500, (bool*)&rv).split(',');
      qDebug() << answer;
      if (answer.count() == 2){
         bool isOk = false;
         int answeredChCount = answer.at(1).toInt(&isOk);
         if (answer.at(0).indexOf('1') == 0 && isOk){
            rv = answeredChCount;
            qDebug() << "channelCount answer:" << answeredChCount;
            }
         }
      }
   return rv;
   }

int MainWindow::getCountrateChCount()
   {
   int rv = 0;
   for(auto a : ttOperations)
      if (a->getSyncStage() == SS_Done && a->getIsCountrateNeeded())
         rv++;
   qDebug() << "getcountrate ch count" << rv;
   return rv;
   }

int MainWindow::getDumpChannelsCount()
   {
   int rv = 0;
   for(auto a : ttOperations)
      if (a->getSyncStage() == SS_Done && a->getIsDumpNeeded())
         rv++;
   if (channelsToDump.count() != rv && ui->TB_DumpFile->isChecked())
      recieveDumpClick(true);
   return rv;
   }

QMap<int,double> MainWindow::getCountrate(int &ts)
   {
   QMap<int,double> rv;
   QMap<TTOpExchangeWrapper*, double> valueMap;
   bool ok = false, ok2 = false, okLong = false;
   QString msg = QString("TAG:CRT?\r\n");
   QString answer = tcpInterface->tcpIpQuery(msg, 1000, &ok);
   qDebug() << answer;

   //   QString ports;
   //   QString values;
   //   for(auto a : ttOperations.uniqueKeys())
   //      ports.append(QString("%1|").arg(QString::number(a)));
   //   ports.chop(1);
   //   for(int i = 0; i < ttOperations.uniqueKeys().count(); i++)
   //      values.append(QString("%1|")
   //                    .arg(QString::number(QRandomGenerator::global()->generate()%1000+i*1000)));
   //   values.chop(1);

   //   QString answer = QString("%1,%2,%3")
   //                    .arg(QString::number(QDateTime(QDate(2022,1,1),QTime(0,0,0)).msecsTo(QDateTime::currentDateTime())))
   //                    .arg(ports)
   //                    .arg(values);
   //   ok = true;

   if (ok){
      ok = false;
      QStringList tempLines = answer.split(",");
      if (tempLines.count() == 3){
         QString longPart = tempLines.at(0);
         qint64 ts = longPart.toULongLong(&okLong);
         qDebug() << longPart.toULongLong(&okLong) << ts;
         QStringList indexes = tempLines.at(1).split("|");
         QStringList values = tempLines.at(2).split("|");
         if (indexes.count() == values.count()){
            for(int i = 0; i < indexes.count(); i++){
               int index = indexes.at(i).toInt(&ok);
               double value = values.at(i).toDouble(&ok2);
               if (ok && ok2 && okLong){
                  rv.insert(index,value);
                  valueMap.insert(ttOperations.value(index),value);
                  ok = true;
                  }
               else
                  ok = false;
               }
            storage.writeData(valueMap,ts);
            plot->updateData();
            }
         }
      }
   return rv;
   }

void MainWindow::findBlocksToSync()
   {
   blocksToSync.clear();
   for(auto a : ttOperations.uniqueKeys()){
      if (ttOperations.value(a)->getSyncStage() != SS_Done){
         if (isSiblingsConfirmed(ttOperations.value(a))){
            ttOperations.value(a)->validate();
            if (ttOperations.value(a)->getSyncStage() == SS_Ready){
               if (ttOperations.value(a)->getTtop() == TTOP_SourceChannel)
                  blocksToSync.prepend(ttOperations.value(a));
               else
                  blocksToSync.append(ttOperations.value(a));
               }
            }
         else
            ttOperations.value(a)->setSyncStage(SS_NotReady);
         }
      }
   }

bool MainWindow::isSiblingsConfirmed(TTOpExchangeWrapper* node)
   {
   bool rv = true;
   for(auto a : node->getInputChannels()){
      if (ttOperations.contains(a))
         rv &= ttOperations.value(a)->getIsConfirmed();
      else rv = false;
      }
   return rv;
   }

bool MainWindow::isOutput(QPair<int, int> pair)
   {
   return pair.second == -1;
   }

int MainWindow::getPortNumber(QPair<int, int> pair)
   {
   return ttOperations.value(pair.first)->getInputChannels().at(pair.second);
   }

void MainWindow::recievePortClick(int id, int portIdx)
   {
   QPair<int,int> currPair = {id,portIdx};
   qDebug() << currPair;

   if (clickedPort.first == 0 //не с чем соединять
       || clickedPort.first == id //клик на тот же блок
       || isOutput(clickedPort) == isOutput(currPair)) //пины одинакового типа (типа вход-вход)
      {
      clickedPort = currPair;

      if (ttOperations.value(id)->getTtop() == TTOP_SourceChannel)
         {
         scene->sourceOutMenu->update();
         scene->sourceOutMenu->popup(QCursor::pos());
         }
      else
         if (isOutput(clickedPort)){
            PortLabel::isHighlighted = 1;
            scene->update();
            }
         else
            {
            PortLabel::isHighlighted = 2;
            scene->update();
            }
      }
   else //если мы тут то это правильная пара пинов и их надо соединить.
      if (!isOutput(currPair)){
         qDebug() <<"prepeared pairs" << currPair << clickedPort;
         ttOperations.value(currPair.first)->replaceLinkByIdx(currPair.second,clickedPort.first);
         blocks.value(currPair.first)->updateInputs();
         scene->setLinksReconfigured(true);
         resetHighlight();
         }
      else
         {
         qDebug() << clickedPort << currPair;
         ttOperations.value(clickedPort.first)->replaceLinkByIdx(clickedPort.second,currPair.first);
         scene->setLinksReconfigured(true);
         blocks.value(clickedPort.first)->updateInputs();
         resetHighlight();
         }
   }

void MainWindow::recieveBlockClick(int id)
   {
   QPair<int,int> currPair = {id,-1};

   if (clickedPort.first != 0
       && clickedPort.first != id
       && isOutput(clickedPort))
      {

      ttOperations.value(currPair.first)->addLink(clickedPort.first);
      blocks.value(currPair.first)->updateInputs();
      scene->setLinksReconfigured(true);
      qDebug() << currPair << clickedPort;
      resetHighlight();
      }
   }

void MainWindow::recieveDumpClick(bool isChecked)
   {
   if (isChecked){
      channelsToDump.clear();
      for(auto a : ttOperations.uniqueKeys())
         if (ttOperations.value(a)->getIsDumpNeeded())
            channelsToDump.append(a);
      if (channelsToDump.isEmpty())
         ui->TB_DumpFile->setChecked(false);
      else
         needToUpdateDumpChannels = true;
      }
   else
      needToUpdateDumpChannels = false;
   }

void MainWindow::resetHighlight()
   {
   PortLabel::isHighlighted = 0;
   scene->update();
   clickedPort = {0,0};
   }

void MainWindow::recieveSyncStatusChanged(SynchronizationStage stage)
   {
   if (stage == SS_Ready)
      if(getCommState() > SCS_TaggerConnection)
         setCommState(SCS_SyncBlocks);
   }

void MainWindow::recieveIsCountrateNeededChanged(bool isNeeded)
   {
   //   qDebug() << "countrateNeeded: " << isNeeded;
   if ((getCommState() == SCS_GetCountrate || getCommState() == SCS_Idle) &&
       (isServerConnected && isTaggerConnected))
      setCommState(SCS_SyncCountRateParams);
   //   qDebug() << "commstate: " << (int)getCommState();
   }

void MainWindow::processSourceMenuAction(QAction *act)
   {
   int newChannel = TTOpExchangeWrapper::stringToSourceCh(act->text());

   ttOperations.insert(newChannel,ttOperations.take(clickedPort.first));
   blocks.insert(newChannel,blocks.take(clickedPort.first));

   for(int i = 0; i < ttOperations.values().count(); i++){
      qDebug() << ttOperations.values().at(i)->getOutChannel() << ttOperations.values().at(i)->getTtop();
      ttOperations.values()[i]->replaceLinkByNumber(clickedPort.first, newChannel);
      }
   clickedPort = {0,0};
   }

void MainWindow::updateStatus()
   {
   if (!isServerConnected){
      ui->L_Status->setStyleSheet("QLabel { background-color : red; }");
      ui->L_Status->setToolTip("Server: Not Connected\nTagger: Not Connected");
      }
   else {
      if (!isTaggerConnected){
         ui->L_Status->setStyleSheet("QLabel { background-color : yellow; }");
         ui->L_Status->setToolTip("Server: Connected\nTagger: Not Connected");
         }
      else{
         ui->L_Status->setStyleSheet("QLabel { background-color : green; }");
         ui->L_Status->setToolTip("Server: Connected\nTagger: Connected");
         }
      }
   }

void MainWindow::save()
   {
   QSettings settings("Tirphotonics","TTProcessingSoftware");
   settings.clear();
   settings.beginWriteArray("BlocksLayout",blocks.uniqueKeys().count());
   int idx = 0;
   for (auto key : blocks.uniqueKeys()) {
      settings.setArrayIndex(idx++);
      settings.setValue("ID",key);
      settings.beginGroup("Geometry");
      QRectF widgetRect = blocks.value(key)->geometry();
      settings.setValue("x",widgetRect.x());
      settings.setValue("y",widgetRect.y());
      settings.setValue("w",widgetRect.width());
      settings.setValue("h",widgetRect.height());
      settings.endGroup();

      settings.beginGroup("TTOpStruct");
      settings.setValue("OperationType",(int)(ttOperations.value(key)->getTtop()));
      //      qDebug() << ttOperations.value(key)->getTtop();
      settings.beginWriteArray("InputChannels",ttOperations.value(key)->getInputChannels().count());
      int idx2 = 0;
      for(auto channel : ttOperations.value(key)->getInputChannels()){
         settings.setArrayIndex(idx2++);
         settings.setValue("Channel",channel);
         }
      settings.endArray();

      settings.beginWriteArray("Parameters",ttOperations.value(key)->getParameters().count());
      int idx3 = 0;
      for(auto parameter : ttOperations.value(key)->getParameters()){
         settings.setArrayIndex(idx3++);
         settings.setValue("Parameter",parameter);
         }
      settings.endArray();

      settings.endGroup();
      }
   settings.endArray();
   }

void MainWindow::load()
   {
   QSettings settings("Tirphotonics","TTProcessingSoftware");
   int arrayLen = settings.beginReadArray("BlocksLayout");
   if (blocks.uniqueKeys().count() > 0 &&
       blocks.uniqueKeys().count() != arrayLen)
      QMessageBox::question(this,
                            QString("Различие операций"),
                            QString("Набор цепочек операций на сервере и клиненте отличается.\n"
                                    "Какой набор оперцаий нужно использовать?"),
                            QString("Сервер"),
                            QString("Клиент"));

   for (int i = 0; i < arrayLen; i++) {
      settings.setArrayIndex(i);
      int id = settings.value("ID").toInt();

      settings.beginGroup("Geometry");
      QRectF widgetGeometry;
      widgetGeometry.setX(settings.value("x").toDouble());
      widgetGeometry.setY(settings.value("y").toDouble());
      widgetGeometry.setWidth(settings.value("w").toDouble());
      widgetGeometry.setHeight(settings.value("h").toDouble());
      settings.endGroup();

      settings.beginGroup("TTOpStruct");
      TTOpExchangeWrapper* tmpTTStruct = new TTOpExchangeWrapper();
      tmpTTStruct->setOutChannel(id);
      tmpTTStruct->setTtop((TTOperationType)settings.value("OperationType").toInt());

      int count2 = settings.beginReadArray("InputChannels");
      QList<int> tempChannels;
      for(int idx2 = 0; idx2 < count2; idx2++){
         settings.setArrayIndex(idx2);
         tempChannels.append(settings.value("Channel").toInt());
         tmpTTStruct->setInputChannels(tempChannels);
         }
      settings.endArray();

      int count3 = settings.beginReadArray("Parameters");
      QList<double> tempParameters;
      for(int idx3 = 0; idx3 < count3; idx3++){
         settings.setArrayIndex(idx3);
         tempParameters.append(settings.value("Parameter").toDouble());
         tmpTTStruct->setParameters(tempParameters);
         }
      if (tempParameters.isEmpty())
         tmpTTStruct->setParameters({0,0,0});
      settings.endArray();

      settings.endGroup();
      //      qDebug() << tmpTTStruct->toString(*tmpTTStruct);

      addBlock(tmpTTStruct,widgetGeometry);
      }
   settings.endArray();
   }

void MainWindow::keyPressEvent(QKeyEvent *event)
   {
   if (event->key() == Qt::Key_Escape){
      resetHighlight();
      }
   QMainWindow::keyPressEvent(event);
   }

GraphicsViewZoom::GraphicsViewZoom(QGraphicsView* view)
  : QObject(view), _view(view)
{
  _view->viewport()->installEventFilter(this);
  _view->setMouseTracking(true);
  _modifiers = Qt::ControlModifier;
  _zoom_factor_base = 1.0015;
}

void GraphicsViewZoom::gentleZoom(double factor) {
  _view->scale(factor, factor);
  _view->centerOn(target_scene_pos);
  QPointF delta_viewport_pos = target_viewport_pos - QPointF(_view->viewport()->width() / 2.0,
                                                             _view->viewport()->height() / 2.0);
  QPointF viewport_center = _view->mapFromScene(target_scene_pos) - delta_viewport_pos;
  _view->centerOn(_view->mapToScene(viewport_center.toPoint()));
  emit zoomed();
}

void GraphicsViewZoom::setModifiers(Qt::KeyboardModifiers modifiers) {
  _modifiers = modifiers;

}

void GraphicsViewZoom::setZoomFactorBase(double value) {
  _zoom_factor_base = value;
}

bool GraphicsViewZoom::eventFilter(QObject *object, QEvent *event) {
  if (event->type() == QEvent::MouseMove) {
    QMouseEvent* mouse_event = static_cast<QMouseEvent*>(event);
    QPointF delta = target_viewport_pos - mouse_event->pos();
    if (qAbs(delta.x()) > 5 || qAbs(delta.y()) > 5) {
      target_viewport_pos = mouse_event->pos();
      target_scene_pos = _view->mapToScene(mouse_event->pos());
    }
  } else if (event->type() == QEvent::Wheel) {
    QWheelEvent* wheel_event = static_cast<QWheelEvent*>(event);
    if (QApplication::keyboardModifiers() == _modifiers) {
      if (wheel_event->orientation() == Qt::Vertical) {
        double angle = wheel_event->angleDelta().y();
        double factor = qPow(_zoom_factor_base, angle);
        gentleZoom(factor);
        return true;
      }
    }
  }
  Q_UNUSED(object)
  return false;
}
