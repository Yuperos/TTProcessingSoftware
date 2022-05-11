#ifndef BLOCKHUB_H
#define BLOCKHUB_H

#include <QObject>

#include "ttgraphscene.h"
#include "swabiancontrol.h"
#include "channelblock.h"
#include "screenmenu.h"

class BlockHub : public QObject
   {
   Q_OBJECT
public:


   explicit BlockHub(QObject *parent = nullptr);

   void createOperation(QString());

//   TTGraphScene *getScene() const;
//   void setScene(TTGraphScene *value);

signals:
   };

#endif // BLOCKHUB_H
