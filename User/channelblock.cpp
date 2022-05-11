#include <QDebug>
#include <cmath>
#include <QMouseEvent>

#include "channelblock.h"
#include "ui_channelblock.h"
#include "portlabel.h"


void ChannelBlock::setHeader(TTOpExchangeWrapper *value)
   {
   header = value;

   ui->TB_Countrate->setStyleSheet("QToolButton { }");
   ui->TB_SetChannel->setHidden(true);

   updateInputs();
   updateOutputs();
   updateTTOperation();

   connect(header, &TTOpExchangeWrapper::syncStageChanged,this,&ChannelBlock::recieveSyncStatusChanged);
   connect(header, &TTOpExchangeWrapper::inputChannelsChanged, this, &ChannelBlock::updateInputs);
   connect(header, &TTOpExchangeWrapper::outChannelChanged, this, &ChannelBlock::updateOutputs);
   connect(header, &TTOpExchangeWrapper::countrateValueChanged,this, &ChannelBlock::recieveCountrateValue);
   connect(ui->TB_Countrate, &QToolButton::toggled,this,&ChannelBlock::recieveCountrateClick);
   connect(ui->TB_DumpFile, &QToolButton::toggled,this,&ChannelBlock::recieveDumpClick);
   header->setSyncStage(SS_Off);

   for(int i = 0; i<parametersHolder.count(); i++){
      connect(parametersHolder.at(i).second,qOverload<int>(&QSpinBox::valueChanged),[this,i](int tmpVal){recieveParameterChanged(i,tmpVal);});
      }
   header->validate();
   }

void ChannelBlock::setStyleHelper(QWidget *widget, QStyle *style)
   {
   widget->setStyle(style);
   widget->setPalette(style->standardPalette());
   const QObjectList children = widget->children();
   for (QObject *child : children) {
      if (QWidget *childWidget = qobject_cast<QWidget *>(child))
         setStyleHelper(childWidget, style);
      }
   }

void ChannelBlock::recieveCountrateClick(bool isChecked)
   {
   header->setIsCountrateNeeded(isChecked);
   if (!isChecked)
      ui->L_Countrate->clear();
   }

void ChannelBlock::recieveDumpClick(bool isChecked)
   {
   header->setIsDumpNeeded(isChecked);
   }

void ChannelBlock::recieveChannelClicked(int channel)
   {
   emit portClicked(header->getOutChannel(),channel);
   }

void ChannelBlock::recieveParameterChanged(int idx, double value)
   {
   header->setParameter(idx,value);
   }

void ChannelBlock::recieveSyncStatusChanged(SynchronizationStage stage)
   {
   switch (stage) {
      case SS_Off:      {
         ui->TB_SyncIndicator->setStyleSheet("QToolButton { background-color : lightGray; }");
         ui->TB_SyncIndicator->setToolTip("Sync: Off");
         } break;
      case SS_NotReady: {
         ui->TB_SyncIndicator->setStyleSheet("QToolButton { background-color : Red; }");
         ui->TB_SyncIndicator->setToolTip("Sync: Not Ready");
         } break;
      case SS_Ready:    {
         ui->TB_SyncIndicator->setStyleSheet("QToolButton { background-color : Orange; }");
         ui->TB_SyncIndicator->setToolTip("Sync: Ready");
         } break;
      case SS_Transfer: {
         ui->TB_SyncIndicator->setStyleSheet("QToolButton { background-color : Yellow; }");
         ui->TB_SyncIndicator->setToolTip("Sync: Transfer Data");
         } break;
      case SS_Done:     {
         ui->TB_SyncIndicator->setStyleSheet("QToolButton { background-color : Green; }");
         ui->TB_SyncIndicator->setToolTip("Sync: Done");
         } break;
      default: setStyleSheet(QString());
      }
   }

void ChannelBlock::recieveCountrateValue(double value)
   {
   ui->L_Countrate->setText(QString::number(value));
   }

void ChannelBlock::dumpDisable(bool value)
   {
   ui->TB_DumpFile->setDisabled(value);
   }

void ChannelBlock::mousePressEvent(QMouseEvent *event)
   {
   QWidget::mousePressEvent(event);
   if (event->isAccepted() == false){
      if (event->button() == Qt::RightButton)
         setFocus();
      else{
         qDebug() << "blockClicked";
         if (!event->isAccepted())
            emit blockClicked(header->getOutChannel());
         }
      }
   }

ChannelBlock::ChannelBlock(TTOpExchangeWrapper *nHeader, QWidget *parent) :
   QWidget(parent),
   ui(new Ui::ChannelBlock)
   {
   ui->setupUi(this);
   parametersHolder.append({ui->L_Param1,ui->SB_Param1});
   paramUnits.append(ui->L_Param1_units);
   parametersHolder.append({ui->L_Param2,ui->SB_Param2});
   paramUnits.append(ui->L_Param2_units);
   parametersHolder.append({ui->L_Param3,ui->SB_Param3});
   paramUnits.append(ui->L_Param3_units);
   setHeader(nHeader);


   QStyle *style = QStyleFactory::create(QApplication::style()->objectName());
   if (style){
      setStyleHelper(this, style);
      }
   }

void ChannelBlock::updateTTOperation()
   {
   switch (header->getTtop()) {
      case _TTOP_empty: return;
      case TTOP_SourceChannel:{
         QString text = QString("Src ch%1 %2")
                        .arg(int(fabs(header->getOutChannel())))
                        .arg((header->getOutChannel()<0)?"R":"F");
         ui->TB_SetChannel->setVisible(true);
         setWindowTitle(text);
         } break;
      case TTOP_Coincendence:{
         setWindowTitle("Coincidence");
         } break;
      case TTOP_GatedChannel:{
         setWindowTitle("Gated");
         } break;
      case TTOP_DelayedChannel:{
         setWindowTitle("Delay");
         } break;
      case _TTOP_count:
         break;
      }
   updateParamList();
   }

ChannelBlock::~ChannelBlock()
   {
   delete ui;
   }

void ChannelBlock::updateInputs()
   {
   header->setIsConfirmed(false);
   if (inputLabels.isEmpty() && !ui->VL_Inputs->layout()->isEmpty()||
       (!ui->VL_Inputs->layout()->isEmpty() && ui->VL_Inputs->layout()->count() > header->getInputChannels().count())||
       (!inputLabels.isEmpty() && ui->VL_Inputs->layout()->count() != inputLabels.count())){
      clearLayout(ui->VL_Inputs->layout());
      if (!inputLabels.isEmpty())
         inputLabels.clear();
      }
   for(int i = 0; i < header->getInputChannels().count(); i++){
      if (i==inputLabels.count()){
         PortLabel *tempLabel = new PortLabel(QString::number(header->getInputChannels().at(i)),1,this);
         inputLabels.append(tempLabel);
         connect(tempLabel,&PortLabel::channelLeftClicked,this,[=](){this->recieveChannelClicked(i);});
         ui->VL_Inputs->addWidget(tempLabel);
         }
      else
         if(inputLabels.at(i)->getPortNumber() != header->getInputChannels().at(i))
            inputLabels.at(i)->setPortNumber(header->getInputChannels().at(i));
      }

   }

void ChannelBlock::updateOutputs()
   {
   if (outputLabel == nullptr && !ui->VL_Outputs->layout()->isEmpty()){
      clearLayout(ui->VL_Outputs->layout());
      outputLabel = nullptr;
      }
   if (ui->VL_Outputs->layout()->isEmpty()){
      if (outputLabel == nullptr)
         outputLabel = new PortLabel(QString::number(header->getOutChannel()),2,this);
      connect(outputLabel,&PortLabel::channelLeftClicked,this,[=](){this->recieveChannelClicked(-1);});
      ui->VL_Outputs->addWidget(outputLabel);
      }
   else
      outputLabel->setPortNumber(header->getOutChannel());
   }

void ChannelBlock::clearLayout(QLayout *layout)
   {
   QLayoutItem *tempLayoutItem;
   while ((tempLayoutItem = layout->takeAt(0)) != 0){
      if (tempLayoutItem->widget())
         tempLayoutItem->widget()->setParent(nullptr);
      delete tempLayoutItem;
      }
   }

void ChannelBlock::updateGuiParams()
   {
   for(int i = 0; i < parametersHolder.count(); i++){
      if (!parametersHolder.at(i).second->isHidden())
         parametersHolder.at(i).second->setValue(header->getParameters().at(i));
      }
   }

void ChannelBlock::updateParamList()
   {
   switch(header->getTtop()){
      case _TTOP_empty: break;
      case TTOP_SourceChannel:{
         setParamGUI(0,"TriggerLvl","mV",-10000,10000);
         setParamGUI(1,"DeadTime"  ,"ps",-10000,10000);
         setParamGUI(2,"Delay"     ,"ps",-100000,100000);
         } break;
      case TTOP_Coincendence:{
         setParamGUI(0,"Window","ps",-100000,100000);
         setParamGUI(1,"TimeStamp Type","",0,4);
         setParamGUI(2);
         } break;
      case TTOP_GatedChannel:{
         setParamGUI(0,"Gate Source CH");
         setParamGUI(1,"Gate Start CH");
         setParamGUI(2,"Gate Stop CH");
         } break;
      case TTOP_DelayedChannel:{
         setParamGUI(0,"Delay","ps",-100000,100000);
         setParamGUI(1);
         setParamGUI(2);
         } break;
      case _TTOP_count: break;
      }
   updateGuiParams();
   }

void ChannelBlock::updateStatus()
   {
   //      if (header->isConfirmed)

   }

void ChannelBlock::setParamGUI(int idx, QString name, QString units, int min, int max)
   {
   if (idx > 0 && idx > 2) return;
   if (name.isEmpty()){
      parametersHolder.at(idx).first->hide();
      }
   else{
      parametersHolder.at(idx).first->setText(name);
      }
   if (units.isEmpty()){
      paramUnits.at(idx)->hide();
      }
   else{
      paramUnits.at(idx)->setText(units);
      }
   if (min == max && min == 0){
      parametersHolder.at(idx).second->hide();
      }
   else{
      parametersHolder.at(idx).second->setRange(min,max);
      }
   }

QList<QPair<int,QPoint>> ChannelBlock::getInputs()
   {
   QList<QPair<int,QPoint>> rv;
   for (int i = 0; i < ui->VL_Inputs->layout()->count(); i++){
      QWidget *widget= ui->VL_Inputs->layout()->itemAt(i)->widget();
      PortLabel *label = qobject_cast<PortLabel*>(widget);
      if (label != nullptr && label->getPortNumber() != 0)
         rv.append(QPair<int,QPoint>(
                      label->getPortNumber(),
                      QPoint(label->geometry().left(),
                             label->geometry().center().y())));
      }
   return rv;
   }

QList<QPair<int,QPoint>> ChannelBlock::getOutputs()
   {
   QList<QPair<int,QPoint>> rv;
   for (int i = 0; i < ui->VL_Outputs->layout()->count(); i++){
      QWidget *widget= ui->VL_Outputs->layout()->itemAt(i)->widget();
      PortLabel *label = qobject_cast<PortLabel*>(widget);
      if (label != nullptr && label->getPortNumber() != 0)
         rv.append(QPair<int,QPoint>(
                      label->getPortNumber(),
                      QPoint(label->geometry().right(),
                             label->geometry().center().y())));
      }
   return rv;
   }


