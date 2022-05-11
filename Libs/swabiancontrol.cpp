#include "swabiancontrol.h"
#include <QDebug>
#include <QDir>
#include <QDateTime>
#include <cmath>

int TTOpExchangeStruct::maxChannel = 0;
QSet<int> TTOpExchangeStruct::avaliableSourceChannels;

SwabianControl::~SwabianControl(){
   if (tagger != nullptr)
      delete tagger;
   }

QString SwabianControl::getLicenseInfo() const
   {
   return licenseInfo;
   }

void SwabianControl::setLicenseInfo(const QString &value)
   {
   licenseInfo = value;
   }

int SwabianControl::getChannelsCount() const
   {
   return channelsCount;
   }

void SwabianControl::setChannelsCount(int value)
   {
   channelsCount = value;
   }

QList<int> SwabianControl::getCountrateChannelsList() const
   {
   return countrateChannelsList;
   }

int SwabianControl::create(TTOpExchangeStruct ttStruct)
   {
   if (tagger == nullptr)
      return 0;

   int channel = 0;

   switch(ttStruct.getTtop()){
      case TTOP_Coincendence: {
         Coincidence *tempOp = new Coincidence(tagger,ttStruct.getInputChannels().toVector().toStdVector(),
                                               ttStruct.getParameters().at(0),
                                               (CoincidenceTimestamp)ttStruct.getParameters().at(1));
         channel = tempOp->getChannel();
         operations.insert(channel, new TTOperationHeader(tempOp));
         } break;
      case TTOP_GatedChannel:{
         if (ttStruct.getInputChannels().count() < 3) break;
         GatedChannel *tempOp = new GatedChannel(tagger,
                                                 ttStruct.getInputChannels().at(0),
                                                 ttStruct.getInputChannels().at(1),
                                                 ttStruct.getInputChannels().at(2));
         channel = tempOp->getChannel();
         operations.insert(channel, new TTOperationHeader(tempOp));
         } break;
      case TTOP_DelayedChannel: {
         DelayedChannel *tempOp = new DelayedChannel(tagger,ttStruct.getInputChannels().first(),
                                                     ttStruct.getParameters().at(0));
         channel = tempOp->getChannel();
         operations.insert(channel, new TTOperationHeader(tempOp));
         } break;
      default: break;
      }
   return channel;
   }

void SwabianControl::edit(TTOpExchangeStruct ttStruct)
   {
   switch(ttStruct.getTtop()){
      case TTOP_Coincendence: {
         Coincidence* op = reinterpret_cast<Coincidence*>(operations[ttStruct.getOutChannel()]->getOperation());
         op->setCoincidenceWindow(ttStruct.getParameters().at(0));
         } break;
      case TTOP_GatedChannel:{
         } break;
      case TTOP_DelayedChannel: {
         auto op = reinterpret_cast<DelayedChannel*>(operations[ttStruct.getOutChannel()]->getOperation());
         op->setDelay(ttStruct.getParameters().at(0));
         } break;
      default: break;
      }
   }

bool SwabianControl::configSourceChannel(TTOpExchangeStruct params)
   {
   bool rv = false;
   if (tagger != nullptr && params.getTtop() == TTOP_SourceChannel){
      tagger->setTriggerLevel(params.getOutChannel(),((double)params.getParameters().at(0))/1000.);
      tagger->setDeadtime(params.getOutChannel(),params.getParameters().at(1));
      tagger->setInputDelay(params.getOutChannel(),params.getParameters().at(2));
      rv = true;
      }
   return rv;
   }

int SwabianControl::syncOperation(TTOpExchangeStruct params)
   {
   int rv = params.getOutChannel();
   bool editable = false;

   if (params.getTtop() != TTOP_SourceChannel && !operations.isEmpty()){
      if (operations.contains(params.getOutChannel()))
         editable = operations.value(params.getOutChannel())->getOperationParameters().editableDifference(params);
      if (!editable)
         removeOperation(params.getOutChannel());
      }

   if (tagger != nullptr){
      if(params.getTtop() == TTOP_SourceChannel){
         int outChanel = params.getOutChannel();
         double trigVoltage = ((double)params.getParameters().at(0))/1000.;
         tagger->setTriggerLevel(outChanel,trigVoltage);
         tagger->setDeadtime(outChanel,params.getParameters().at(1));
         tagger->setInputDelay(outChanel,params.getParameters().at(2));
         channelsUsed.insert(outChanel);
         }
      else{
         if (editable)
            edit(params);
         else{
            rv = create(params);
            channelsUsed.insert(rv);
            }
         }
      }
   return rv;
   }

void SwabianControl::removeOperation(int id)
   {
   if (!operations.isEmpty()){
      auto list = operations.uniqueKeys();
      for(auto a: list){
         int tempIdx = operations.value(a)->getOperationParameters().getInputChannels().indexOf(id);
         if (tempIdx != -1){
            if (countrateChannelsList.contains(id) && cr != nullptr){
               cr->stop();
               countrateChannelsList.removeAt(id);
               }
            if (tempIdx !=  id) removeOperation(tempIdx);
            }
         }
      }
   }

bool SwabianControl::setupCountrate(QList<int> channels)
   {
   if (tagger != nullptr){
      countrateChannelsList.clear();
      countrateChannelsList.append(channels);
      if (cr != nullptr){
         cr->stop();
         delete cr;
         }
      if (channels.count() != 1 || channels.at(0) != 0){
         qDebug() << channels.count() << channels.toVector().toStdVector();
         cr = new Countrate(tagger,channels.toVector().toStdVector());
         //         cr->startFor(1e12);
         return true;
         }
      }
   return false;
   }

bool SwabianControl::setupSaveRaw(QList<int> channels)
   {
   bool rv = false;
   if (writer != nullptr){
      //      writer->stop();
      delete writer;
      }

   if (channels.count() > 0){
      QString directory = QString("%1/RawData/DumpAt_%2")
                          .arg(QDir::currentPath())
                          .arg(QDateTime::currentDateTime().date().toString("yyyy_MM_dd"));
      QDir dir(directory);
      if (!dir.exists())
         if (dir.mkpath(".")){
            QString filename = QString("/StartAt_%1_Ch.ttbin").arg(QDateTime::currentDateTime().time().toString("hh_mm_ss"));
            QString absPath = QString("%1%2")
                              .arg(directory)
                              .arg(filename);

            qDebug()<< absPath;

            QFile testFile(absPath.append("_test"));
            if (testFile.open(QIODevice::WriteOnly)){
               testFile.remove();
               writer = new FileWriter(tagger,absPath.toStdString(),channels.toVector().toStdVector());
               //      writer->start();
               rv = true;
               }
            }
      }
   return rv;
   }

QList<double> SwabianControl::getCountrateValues()
   {
   std::vector<double> data;

   if(tagger != nullptr && cr != nullptr){
      cr->getData([&data](size_t size) {
         data.resize(size);
         return data.data();
         });
      cr->clear();
      }
   return QList<double>::fromVector(QVector<double>::fromStdVector(data));
   }

SwabianControl::SwabianControl()
   {
   }

int SwabianControl::connectTimeTagger()
   {
   if (tagger == nullptr){
      std::vector<std::string> taggers = scanTimeTagger();
      if (taggers.empty()) {
         std::cout << std::endl << "No time tagger found." << std::endl << "Please attach a Time Tagger." << std::endl;
         }
      else{
         tagger = createTimeTagger();
         licenseInfo = QString::fromStdString(tagger->getLicenseInfo());
         auto channelsList = tagger->getChannelList();
         channelsCount = *std::max_element(channelsList.begin(),channelsList.end());
         }
      }
   else{
      tagger->reset();
      for(auto a : channelsUsed)
         if (operations.contains(a)){
            auto temp = operations[a];
            operations.remove(a);
            delete temp;
            }
      }

   return channelsCount;
   }

TTOpExchangeStruct TTOperationHeader::getOperationParameters() const
   {
   return operationParameters;
   }

void TTOperationHeader::setOperationParameters(const TTOpExchangeStruct &value)
   {
   operationParameters = value;
   }

IteratorBase *TTOperationHeader::getOperation() const
   {
   return operation;
   }

TTOperationHeader::TTOperationHeader(IteratorBase *value):
   operation(value)
   {
   }

TTOperationHeader::~TTOperationHeader()
   {
   }

bool TTOpExchangeStruct::addLink(int id)
   {
   bool rv = false;
   if ((ttop == TTOP_GatedChannel && inputChannels.count() < 3)
       || (ttop == TTOP_DelayedChannel && inputChannels.count() < 1)
       ||ttop == TTOP_Coincendence){
      inputChannels.append(id);
      rv = true;
      }


   //   if (ttop == TTOP_GatedChannel){
   //      if (inputChannels.count() > 2 || clickedIdx < -1024)
   //         inputChannels.replace(inputChannels.indexOf(clickedIdx),id);
   //      else
   //         inputChannels.append(id);
   //      }
   //   else if (ttop != TTOP_Coincendence)
   //      inputChannels.clear();
   //   else if (!inputChannels.isEmpty() && inputChannels.last() < -1024)
   //      inputChannels.removeLast();
   //   if (inputChannels.indexOf(id) == -1)12
   //      inputChannels.append(id);
   return rv;
   }

bool TTOpExchangeStruct::replaceLinkByNumber(int numBefore, int after)
   {
   if (outChannel == numBefore) {
      outChannel = after;
      return true;
      }
   else
      {
      int idx = inputChannels.indexOf(numBefore);
      if (idx != -1){
         inputChannels.replace(idx,after);
         return true;
         }
      }
   return false;
   }

bool TTOpExchangeStruct::replaceLinkByIdx(int idxBefore, int after)
   {
   if (idxBefore == -1) {
      outChannel = after;
      return true;
      }
   else
      {
      inputChannels.replace(idxBefore,after);
      return true;
      }
   return false;
   }

void TTOperationHeader::setIsMarkedForDelete(bool value)
   {
   isMarkedForDelete = value;
   }

void TTOpExchangeStruct::check()
   {
   if (ttop == TTOP_SourceChannel)
      inputChannels.clear();
   }

InputErrorTypes TTOpExchangeStruct::validateStruct()
   {
   int tempRv = _IE_none;
   for(auto input : getInputChannels())
      if (input < -maxChannel)
         tempRv |= IE_Inputs;
   if (getTtop() == TTOP_GatedChannel && getInputChannels().toSet().count() < 3)
      tempRv |= IE_Inputs;
   if (getTtop() == TTOP_SourceChannel
       && (maxChannel < 0
           || fabs(getOutChannel()) == 0
           || fabs(getOutChannel()) > maxChannel))
      tempRv |= IE_SourceOutput;
   return (InputErrorTypes)tempRv;
   }

void TTOpExchangeStruct::setInputChannels(const QList<int> &value)
   {
   inputChannels = value;
   }


void TTOpExchangeStruct::setParameters(const QList<double> &value)
   {
   parameters = value;
   while(parameters.count() < 3)
      parameters.append(0);
   }

bool TTOpExchangeStruct::setTtop(const int &value)
   {
   if(_TTOP_empty < value && _TTOP_count > value){
      setTtop((TTOperationType)value);
      return true;
      }
   return false;
   }

bool TTOpExchangeStruct::setOutChannel(int value)
   {
   bool rv = false;
   if (outChannel != value){
      if (ttop == TTOP_SourceChannel){
         if (fabs(outChannel) <= 18 && outChannel != 0) {
            avaliableSourceChannels.insert(fabs(outChannel));
            }
         if (avaliableSourceChannels.contains(fabs(value))) {
            avaliableSourceChannels.remove(fabs(value));
            }
         }
      outChannel = value;
      }
   return rv;
   }

bool TTOpExchangeStruct::setTtop(const TTOperationType &value)
   {
   bool rv = false;
   if (ttop != value){
      ttop = value;
      rv= true;
      }
   return rv;
   }

bool TTOpExchangeStruct::setIsConfirmed(bool value)
   {
   bool rv = false;
   if (isConfirmed != value){
      isConfirmed = value;
      rv = true;
      }
   return rv;
   }

bool TTOpExchangeStruct::setParameter(int idx, double value)
   {
   if (parameters.count() > idx){
      parameters[idx] = value;
      return true;
      }
   else
      return false;
   }

TTOpExchangeStruct::TTOpExchangeStruct():
   outChannel(0),
   ttop(_TTOP_empty),
   isConfirmed(false)
   {
   inputChannels.clear();
   parameters.clear();
   init();
   }

TTOpExchangeStruct::TTOpExchangeStruct(int newOutChannel, TTOperationType nTtop, QList<int> nInputChannels, QList<double> nParameters):
   outChannel(newOutChannel),
   ttop(nTtop),
   inputChannels(nInputChannels),
   parameters(nParameters),
   isConfirmed(false)
   {
   init();
   }

void TTOpExchangeStruct::init()
   {
   while(parameters.count() < 3)
      parameters.append(0);
   initAvaliableSourceChannels();
   }

void TTOpExchangeStruct::initAvaliableSourceChannels(int nMaxChannel)
   {
   if (TTOpExchangeStruct::maxChannel != nMaxChannel){
      if (nMaxChannel>0)
         TTOpExchangeStruct::maxChannel = nMaxChannel;
      TTOpExchangeStruct::avaliableSourceChannels.clear();
      for (int i = 1;i < TTOpExchangeStruct::maxChannel+1; i++)
         TTOpExchangeStruct::avaliableSourceChannels.insert(i);
      }
   }

bool TTOpExchangeStruct::operator==(const TTOpExchangeStruct &other)
   {
   bool rv =(outChannel == other.outChannel
             && ttop == other.ttop
             && inputChannels.count() == other.inputChannels.count());

   if (rv == true)
      for(int i = 0; i<inputChannels.count() && rv==true; i++)
         rv &= (inputChannels.at(i) == other.inputChannels.at(i));

   if (rv == true)
      for(int i = 0; i<parameters.count() && rv==true; i++)
         rv &= fabs(parameters.at(i) - other.parameters.at(i)) < 0.0001;
   return rv;
   }

bool TTOpExchangeStruct::editableDifference(const TTOpExchangeStruct &other)
   {
   bool rv =(outChannel == other.outChannel
             && ttop == other.ttop
             && inputChannels.count() == other.inputChannels.count());

   if (rv == true)
      for(int i = 0; i<inputChannels.count() && rv==true; i++)
         if (other.ttop == TTOP_GatedChannel)
            rv &= (inputChannels.at(i) == other.inputChannels.at(i));
         else
            rv &= (other.inputChannels.contains(inputChannels.at(i)));
   if (rv == true){
      if (other.ttop == TTOP_Coincendence)
         rv &= getParameters().at(1) == other.getParameters().at(1);
      else if (other.ttop == TTOP_GatedChannel){
         rv &= getParameters().at(0) == other.getParameters().at(0) &&
               getParameters().at(1) == other.getParameters().at(1);
         }
      }
   return rv;
   }

TTOpExchangeStruct::operator QString()
   {
   QString inputsArray,paramArray;
   if (inputChannels.count() > 0)
      for(int i = 0; i< inputChannels.count(); i++){
         inputsArray.append(QString::number(inputChannels.at(i)));
         if (i+1 < inputChannels.count())
            inputsArray.append("|");
         }
   if (parameters.count() > 0)
      for(int i = 0; i< parameters.count(); i++){
         paramArray.append(QString::number(parameters.at(i)));
         if (i+1 < parameters.count())
            paramArray.append("|");
         }
   return QString("%1,%2,{%3},{%4}").arg(QString::number(outChannel),
                                         QString::number((int)ttop),
                                         inputsArray,
                                         paramArray);
   }

QString TTOpExchangeStruct::toString(TTOpExchangeStruct in)
   {
   QString inputsArray,paramArray;
   if (in.inputChannels.count() > 0)
      for(int i = 0; i< in.inputChannels.count(); i++){
         inputsArray.append(QString::number(in.inputChannels.at(i)));
         if (i+1 < in.inputChannels.count())
            inputsArray.append("|");
         }
   else
      inputsArray.append(" ");

   if (in.parameters.count() > 0)
      for(int i = 0; i< in.parameters.count(); i++){
         paramArray.append(QString::number(in.parameters.at(i)));
         if (i+1 < in.parameters.count())
            paramArray.append("|");
         }
   else
      paramArray.append(" ");
   return QString("%1,%2,{%3},{%4}").arg(QString::number(in.outChannel),
                                         QString::number((int)in.ttop),
                                         inputsArray,
                                         paramArray);
   }

TTOpExchangeStruct TTOpExchangeStruct::fromString(QString str)
   {
   TTOpExchangeStruct rv;
   QStringList temp = str.split(QRegExp(","),Qt::KeepEmptyParts);
   for(int i = 0; i < temp.count(); i++){
      temp[i].remove(QChar('{'));
      temp[i].remove(QChar('}'));
      }
   if (temp.count()>1){
      int tempTTOP = temp.at(1).toInt();
      if (tempTTOP >= 0 && tempTTOP < _TTOP_count){
         rv.outChannel = temp.at(0).toInt();
         rv.ttop = (TTOperationType)tempTTOP;
         for(const auto &a : temp.at(2).split("|",Qt::SkipEmptyParts))
            rv.inputChannels.append(a.toInt());
         rv.parameters.clear();
         for(const auto &a : temp.at(3).split("|",Qt::SkipEmptyParts))
            rv.parameters.append(a.toDouble());
         }
      }
   return rv;
   }

QString TTOpExchangeStruct::toString(TTOperationType ttop)
   {
   QString rv;
   switch (ttop) {
      case TTOP_SourceChannel: {
         rv.append("SourceChannel");
         } break;
      case TTOP_Coincendence: {
         rv.append("Coincendence");
         } break;
      case TTOP_GatedChannel: {
         rv.append("GatedChanel");
         } break;
      case TTOP_DelayedChannel: {
         rv.append("DelayedChannel");
         } break;
      case TTOP_Histogram: {
         rv.append("Histogram");
         } break;
      default: break;
      }
   return rv;
   }

QString TTOpExchangeStruct::toShortString(TTOperationType ttop)
   {
   QString rv;
   switch (ttop) {
      case TTOP_SourceChannel: {
         rv.append("Src");
         } break;
      case TTOP_Coincendence: {
         rv.append("CoinS");
         } break;
      case TTOP_GatedChannel: {
         rv.append("Gate");
         } break;
      case TTOP_DelayedChannel: {
         rv.append("Delay");
         } break;
      case TTOP_Histogram: {
         rv.append("Hist");
         } break;
      default: break;
      }
   return rv;
   }

QString TTOpExchangeStruct::parametersToOutString(TTOpExchangeStruct operation)
   {
   QString rv;
   QStringList params;
   for(int i = 0; i<3; i++)
      params.append(QString::number(operation.getParameters().at(i)));

   switch (operation.getTtop()) {
      case TTOP_SourceChannel: {
         rv = QString("Lvl=%1V DT=%2ps Del=%3ps")
              .arg(params.at(0))
              .arg(params.at(1))
              .arg(params.at(2));
         } break;
      case TTOP_Coincendence: {
         QString type;
         switch((int)operation.getParameters().at(1)){
            case 0: type = "Last"; break;
            case 1: type = "Avg"; break;
            case 2: type = "1st"; break;
            case 3: type = "L_1st"; break;
            }
         rv = QString("W=%1ps Ts=%2")
              .arg(params.at(0))
              .arg(type);
         } break;
      case TTOP_GatedChannel: {
         rv = QString("Src#%1 Start#%2 Stop#%3")
              .arg(QString::number(operation.getInputChannels().at(0)))
              .arg(QString::number(operation.getInputChannels().at(1)))
              .arg(QString::number(operation.getInputChannels().at(2)));
         } break;
      case TTOP_DelayedChannel: {
         rv = QString("Del:%1ps")
              .arg(params.at(0));
         } break;
      default: break;
      }
   return rv;
   }

TTOperationType TTOpExchangeStruct::stringToOpType(QString str)
   {
   TTOperationType rv = _TTOP_empty;
   if(str.indexOf("SourceChannel") == 0)
      rv = TTOP_SourceChannel;
   else if(str.indexOf("Coincendence") == 0)
      rv = TTOP_Coincendence;
   else if(str.indexOf("GatedChanel") == 0)
      rv = TTOP_GatedChannel;
   else if(str.indexOf("DelayedChannel") == 0)
      rv = TTOP_DelayedChannel;
   else if(str.indexOf("Histogram") == 0)
      rv = TTOP_Histogram;
   return rv;
   }

QString TTOpExchangeStruct::ChNumberToString(int channelNumber)
   {
   return QString("%1 %2").arg(channelNumber).arg((channelNumber>0)?"Front":"Rear");
   }

int TTOpExchangeStruct::stringToSourceCh(QString str)
   {
   auto list = str.split(" ");
   int ch = list.at(0).toInt();
   if (list.at(1).indexOf("Rear") == 0)
      ch*=-1;
   return ch;
   }

int TTOpExchangeStruct::getOutChannel() const
   {
   return outChannel;
   }

bool TTOpExchangeStruct::getIsConfirmed() const
   {
   return isConfirmed;
   }

TTOperationType TTOpExchangeStruct::getTtop() const
   {
   return ttop;
   }

QList<int> TTOpExchangeStruct::getInputChannels() const
   {
   return inputChannels;
   }

QList<double> TTOpExchangeStruct::getParameters() const
   {
   return parameters;
   }

int32_t TTOperationHeader::getOperationChannel()
   {
   return operationParameters.getOutChannel();
   }

bool TTOperationHeader::getIsMarkedForDelete() const
   {
   return isMarkedForDelete;
   }


SynchronizationStage TTOpExchangeWrapper::getSyncStage() const
   {
   return syncStage;
   }

bool TTOpExchangeWrapper::getIsCountrateNeeded() const
   {
   return isCounterRateNeeded;
   }

void TTOpExchangeWrapper::setIsCountrateNeeded(bool value)
   {
   if (isCounterRateNeeded != value){
      emit countrateNeededChanged(value);
      isCounterRateNeeded = value;
      }
   }

double TTOpExchangeWrapper::getCountrate() const
   {
   return counterRate;
   }

void TTOpExchangeWrapper::setCountrate(double value)
   {
   if (fabs(counterRate - value) > 0.001){
      counterRate = value;
      emit countrateValueChanged(counterRate);
      }
   }

bool TTOpExchangeWrapper::getIsDumpNeeded() const
   {
   return isDumpNeeded;
   }

void TTOpExchangeWrapper::setIsDumpNeeded(bool value)
   {
   isDumpNeeded = value;
   }

TTOpExchangeWrapper::TTOpExchangeWrapper(QObject* parent):
   TTOpExchangeStruct(),
   QObject(parent)
   {
   }

TTOpExchangeWrapper::TTOpExchangeWrapper(int newOutChannel, TTOperationType nTtop, QList<int> nInputChannels, QList<double> nParameters, QObject *parent):
   TTOpExchangeStruct(newOutChannel,nTtop,nInputChannels,nParameters),
   QObject(parent)
   {
   }

void TTOpExchangeWrapper::setSyncStage(const SynchronizationStage &value)
   {
   if (syncStage != value){
      syncStage = value;
      emit syncStageChanged(syncStage);
      }
   }

void TTOpExchangeWrapper::validate()
   {
   if (!isConfirmed){
      if (validateStruct() == _IE_none)
         setSyncStage(SS_Ready);
      else
         setSyncStage(SS_NotReady);
      }
   }

bool TTOpExchangeWrapper::setTtop(const TTOperationType &value)
   {
   TTOperationType prevVal = ttop;
   if (TTOpExchangeStruct::setTtop(value)){
      emit ttopChanged(prevVal);
      setIsConfirmed(false);
      return true;
      }
   return false;
   }

bool TTOpExchangeWrapper::addLink(int id)
   {
   if (TTOpExchangeStruct::addLink(id)){
      emit inputChannelsChanged(id);
      setIsConfirmed(false);
      return true;
      }
   return false;
   }

bool TTOpExchangeWrapper::replaceLinkByNumber(int numBefore, int after)
   {
   int isOutChannel = (getOutChannel() == numBefore);
   if (TTOpExchangeStruct::replaceLinkByNumber(numBefore,after)){
      if (isOutChannel)
         emit outChannelChanged(numBefore);
      else
         emit inputChannelsChanged(after);
      return true;
      }
   return false;
   }

bool TTOpExchangeWrapper::replaceLinkByIdx(int idxBefore, int after)
   {
   int isOutChannel = (idxBefore == -1);
   if (TTOpExchangeStruct::replaceLinkByIdx(idxBefore,after)){
      if (isOutChannel)
         emit outChannelChanged(getOutChannel());
      else
         emit inputChannelsChanged(after);
      return true;
      }
   return false;
   }


bool TTOpExchangeWrapper::setOutChannel(int value)
   {
   int prevVal = outChannel;
   if (TTOpExchangeStruct::setOutChannel(value)){
      emit outChannelChanged(value);
      setIsConfirmed(false);
      return true;
      }
   return false;
   }

bool TTOpExchangeWrapper::setIsConfirmed(bool value)
   {
   bool prevVal = isConfirmed;
   if (TTOpExchangeStruct::setIsConfirmed(value)){
      emit confirmedChanged(prevVal);
      validate();
      return true;
      }
   return false;
   }

bool TTOpExchangeWrapper::setParameter(int idx, double value)
   {
   if (TTOpExchangeStruct::setParameter(idx,value)){
      emit parametersChanged(idx);
      setIsConfirmed(false);
      return true;
      }
   return false;
   }
