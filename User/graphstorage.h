#ifndef GRAPHSTORAGE_H
#define GRAPHSTORAGE_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QVector3D>
#include <QReadWriteLock>
#include <QFile>
#include "swabiancontrol.h"

class GraphStorage : public QObject
   {
   Q_OBJECT
   
   QList<qint64> reverseTsList;
   QMap<TTOpExchangeWrapper*,QList<QVector3D>> rowsMap;
   QList<TTOpExchangeWrapper*> lookupList;
   QList<QVector3D> dummyList;

   qint64 lastTag;
   qint64 writedDataCount = 0;
   uint32_t valuesCount = 0;
   uint32_t maxValues = 50000;

   QFile outFile;

public:
//   QReadWriteLock fileLock;
//   QReadWriteLock lock;

   explicit GraphStorage(QObject *parent = nullptr);
   
   void writeData(QMap<TTOpExchangeWrapper*, double> valueMap, qint64 ts);
   void appendRow(TTOpExchangeWrapper* block, bool isBulkAppend = false);
   void deleteRow(TTOpExchangeWrapper* block);
   int countInRange(int window);

   void removeData(int rowsToStay);

   QList<QVector3D> &getDataRow(int idx);
   int dataRowsCount();
   qint64 getLastTag() const;
   uint32_t getValuesCount() const;
   void setFile(QString filePath);
   void appendToFile(QMap<TTOpExchangeWrapper*, double> valueMap, qint64 ts);

   void setMaxValues(const uint32_t &value);

   qint64 getWritedDataCount() const;

signals:
   void rowAppended();
   void writedDataCountChanged(int);
   };

#endif // GRAPHSTORAGE_H
