#include "graphstorage.h"
#include <QTextStream>
#include <QDebug>

void GraphStorage::setMaxValues(const uint32_t &value)
   {
   maxValues = value;
   }

qint64 GraphStorage::getWritedDataCount() const
   {
   return writedDataCount;
   }

GraphStorage::GraphStorage(QObject *parent) : QObject(parent)
   {

   }

void GraphStorage::setFile(QString filePath)
   {
   writedDataCount = 0;
   emit writedDataCountChanged(writedDataCount);
   outFile.setFileName(filePath);
   if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)){
      qDebug() << "can't create file";
      return;
      }

   QTextStream outStream(&outFile);
   outStream << "timeStamp (ps);";
   for(auto a: rowsMap.uniqueKeys()){
      QString out = QString("%1[%2] %3;")
            .arg(QString::number(a->getOutChannel()))
            .arg(TTOpExchangeStruct::toShortString(a->getTtop()))
            .arg(TTOpExchangeStruct::parametersToOutString(*a));
      outStream << out;
      }
   outStream << endl;
   outFile.close();
   }

void GraphStorage::appendToFile(QMap<TTOpExchangeWrapper *, double> valueMap, qint64 ts)
   {
   if (outFile.fileName().isEmpty() || !outFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)){
      qDebug() << "can't create file";
      return;
      }

   QTextStream outStream(&outFile);
   outStream << QString::number(ts);
   outStream << ";";
   for(auto a: valueMap.uniqueKeys()){
      outStream << QString::number(valueMap.value(a)) << ";" ;
      writedDataCount++;
      }
   outStream << endl;
   outFile.close();
   emit writedDataCountChanged(writedDataCount);
   }

void GraphStorage::writeData(QMap<TTOpExchangeWrapper *, double> valueMap, qint64 ts)
   {
   lastTag = ts;
   auto sourceSet = rowsMap.uniqueKeys().toSet();
   auto incomingSet = valueMap.uniqueKeys().toSet();
   if (incomingSet.count() > sourceSet.count()){
      for(auto a : incomingSet.subtract(sourceSet))
         appendRow(a,true);
      emit rowAppended();
      sourceSet = rowsMap.uniqueKeys().toSet();
      }

   appendToFile(valueMap,ts);

   reverseTsList.prepend(ts);
   for(int i=0; i < lookupList.count(); i++){
      valuesCount++;
      //         lock.lockForWrite();
      if (incomingSet.contains(lookupList.at(i)))
         rowsMap[lookupList.at(i)].prepend(QVector3D(ts,valueMap.value(lookupList.at(i)),i));
      else
         rowsMap[lookupList.at(i)].prepend(QVector3D(ts,0,i));
      //         lock.unlock();
      }
   if (valuesCount > maxValues)
      removeData(maxValues/rowsMap.uniqueKeys().count());
   }

void GraphStorage::appendRow(TTOpExchangeWrapper *block, bool isBulkAppend)
   {
   //   lock.lockForWrite();
   if (block != nullptr && !rowsMap.contains(block)){
      rowsMap.insert(block,QList<QVector3D>());
      lookupList.append(block);
      if (!isBulkAppend)
         emit rowAppended();
      }
   //   lock.unlock();
   }

void GraphStorage::deleteRow(TTOpExchangeWrapper *block)
   {
   //   lock.lockForWrite();
   if (!rowsMap.isEmpty() && rowsMap.contains(block)){
      valuesCount -= rowsMap.value(block).count();
      rowsMap.remove(block);
      }
   //   lock.unlock();
   }

int GraphStorage::countInRange(int window)
   {
   qint64 rv=0;
   qint64 lastTimeStamp = 0;
   if (reverseTsList.count() > 0)
      lastTimeStamp = reverseTsList.last();
   //   qint64 i = 0;
   while(rv < reverseTsList.count() && lastTimeStamp - reverseTsList.at(rv) < window){
      rv++;
      }
   return rv;
   }

void GraphStorage::removeData(int rowsToStay)
   {
   //   lock.lockForWrite();
   for(auto rowKey : rowsMap.uniqueKeys())
      rowsMap[rowKey].erase(rowsMap[rowKey].begin()+rowsToStay);
   reverseTsList.erase(reverseTsList.begin()+rowsToStay);
   //   lock.unlock();
   }

QList<QVector3D> &GraphStorage::getDataRow(int idx)
   {
   //   lock.lockForRead();
   if (idx < lookupList.count() && rowsMap.contains(lookupList.at(idx))){
      return rowsMap[lookupList.at(idx)];
      }
   else
      return dummyList;
   //   lock.unlock();
   }

int GraphStorage::dataRowsCount()
   {
   return rowsMap.uniqueKeys().count();
   }

qint64 GraphStorage::getLastTag() const
   {
   return lastTag;
   }

uint32_t GraphStorage::getValuesCount() const
   {
   return valuesCount;
   }
