#ifndef SWABIANCONTROL_H
#define SWABIANCONTROL_H

#include <QMap>
#include <QSet>
#include <QObject>
#include "TimeTagger.h"
#include "Iterators.h"

#define Singleton(__CLASSNAME__) \
public: \
static __CLASSNAME__* getInstance() { \
   static __CLASSNAME__ instance; \
   return &instance; \
   } \
private:\
   __CLASSNAME__();\
   ~__CLASSNAME__ (); \
   __CLASSNAME__( const __CLASSNAME__ & ) = delete; \
   __CLASSNAME__ & operator=( __CLASSNAME__ & ) = delete

enum TTOperationType{
   _TTOP_empty = -1,
   TTOP_SourceChannel = 0,
   TTOP_Coincendence,
   TTOP_GatedChannel,
   TTOP_DelayedChannel,
   TTOP_Histogram,
   _TTOP_count,
   };

enum InputErrorTypes{
   _IE_none =        0b000,
   IE_Inputs =       0b001,
   IE_SourceOutput = 0b010,
   IE_Parameters =   0b100,
   };

enum SynchronizationStage{
   SS_Off = 0,
   SS_NotReady,
   SS_Ready,
   SS_Transfer,
   SS_Done,
   };

class TTOpExchangeStruct{
protected:
   bool isConfirmed = false;
   int outChannel;
   TTOperationType ttop;
   QList<int> inputChannels;
   QList<double> parameters;

public:   
   static int maxChannel;
   static QSet<int> avaliableSourceChannels;

   bool setTtop(const int &value);
   void setInputChannels(const QList<int> &value);
   void setParameters(const QList<double> &value);
   virtual bool setTtop(const TTOperationType &value);
   virtual bool setOutChannel(int value);
   virtual bool setIsConfirmed(bool value);
   virtual bool setParameter(int idx, double value);  

   int getOutChannel() const;
   bool getIsConfirmed() const;
   TTOperationType getTtop() const;
   QList<int> getInputChannels() const;
   QList<double> getParameters() const;
   InputErrorTypes validateStruct();

   virtual bool addLink(int id);
   virtual bool replaceLinkByNumber(int numBefore, int after);
   virtual bool replaceLinkByIdx(int idxBefore, int after);
   void check();
   void init();
   static void initAvaliableSourceChannels(int nMaxChannel = 0);

   explicit TTOpExchangeStruct();
   explicit TTOpExchangeStruct(int newOutChannel, TTOperationType nTtop, QList<int> nInputChannels, QList<double> nParameters);

   bool operator==(const TTOpExchangeStruct &other);
   bool editableDifference(const TTOpExchangeStruct &other);
   operator QString();

   static QString toString(TTOpExchangeStruct in);
   static TTOpExchangeStruct fromString(QString str);
   static QString toString(TTOperationType ttop);
   static TTOperationType stringToOpType(QString str);
   static QString ChNumberToString(int channelNumber);
   static int stringToSourceCh(QString str);

   static QString toShortString(TTOperationType ttop);
   static QString parametersToOutString(TTOpExchangeStruct operation);
   };

class TTOpExchangeWrapper: public QObject, public TTOpExchangeStruct
   {
   Q_OBJECT

   SynchronizationStage syncStage;
   bool isCounterRateNeeded = false;
   bool isDumpNeeded = false;
   double counterRate;

public:
   explicit TTOpExchangeWrapper(QObject *parent = nullptr);
   explicit TTOpExchangeWrapper(int newOutChannel, TTOperationType nTtop, QList<int> nInputChannels, QList<double> nParameters, QObject* parent = nullptr);\

signals:
   void confirmedChanged(bool);
   void outChannelChanged(int prev);
   void ttopChanged(TTOperationType);
   void inputChannelsChanged(int);
   void parametersChanged(int);
   void syncStageChanged(SynchronizationStage);
   void countrateNeededChanged(bool);
   void countrateValueChanged(double);

public:
   bool setTtop(const TTOperationType &value) override;
   bool addLink(int id) override;
   bool replaceLinkByNumber(int numBefore, int after);
   bool replaceLinkByIdx(int idxBefore, int after);
   bool setOutChannel(int value) override;
   bool setIsConfirmed(bool value) override;
   bool setParameter(int idx, double value) override;
   void setSyncStage(const SynchronizationStage &value);
   void validate();

   SynchronizationStage getSyncStage() const;
   bool getIsCountrateNeeded() const;
   void setIsCountrateNeeded(bool value);
   double getCountrate() const;
   void setCountrate(double value);
   bool getIsDumpNeeded() const;
   void setIsDumpNeeded(bool value);
   };

class TTOperationHeader
   {
   TTOpExchangeStruct operationParameters;
   IteratorBase* operation;
   bool isMarkedForDelete = false;
   bool isShowCounterRate = false;

public:
   TTOperationHeader(IteratorBase *value);
   ~TTOperationHeader();

   int32_t getOperationChannel();
   bool getIsMarkedForDelete() const;
   void setIsMarkedForDelete(bool value);
   TTOpExchangeStruct getOperationParameters() const;
   void setOperationParameters(const TTOpExchangeStruct &value);
   IteratorBase *getOperation() const;
   };

class SwabianControl
   {
   Singleton(SwabianControl);

   Countrate *cr;
   TimeTagger *tagger;
   FileWriter *writer = nullptr;
   QMap<int,TTOperationHeader*> operations;
   QSet<int> channelsUsed;
   QList<int> countrateChannelsList;

   QString licenseInfo;
   int channelsCount;

public:
   int create(TTOpExchangeStruct ttStruct);
   void edit(TTOpExchangeStruct ttStruct);
   void removeOperation(int id);

   int connectTimeTagger();
   bool configSourceChannel(TTOpExchangeStruct params);
   int syncOperation(TTOpExchangeStruct params);
   bool setupCountrate(QList<int> channels);
   bool setupSaveRaw(QList<int> channels);
   QList<double> getCountrateValues();
   QString getLicenseInfo() const;
   void setLicenseInfo(const QString &value);
   int getChannelsCount() const;
   void setChannelsCount(int value);
   QList<int> getCountrateChannelsList() const;
   };

#endif // SWABIANCONTROL_H
