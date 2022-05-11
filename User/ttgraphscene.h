#ifndef TTGRAPHSCENE_H
#define TTGRAPHSCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QMap>

#include "screenmenu.h"

class TTGraphScene : public QGraphicsScene
   {
   Q_OBJECT

   bool linksReconfigured = true;
public:
   explicit TTGraphScene(QObject *parent = nullptr);

   AddOperationMenu *operationsMenu;
   SetSourceOutMenu *sourceOutMenu;
   QMultiMap<int, QPointF> inputs;
   QMap<int, QPointF> outputs;
   QList<QGraphicsLineItem*> lines;

   void setLinksReconfigured(bool value);

public slots:
   void setupLines();
   void updateLines();
   void getPortConfigurations();
   // QGraphicsScene interface
protected:
   void drawForeground(QPainter *painter, const QRectF &rect) override;
//   void drawBackground(QPainter *painter, const QRectF &rect) override;
//   void drawItems(QPainter *painter, int numItems, QGraphicsItem *items[], const QStyleOptionGraphicsItem options[], QWidget *widget) override;

   // QGraphicsScene interface
protected:
   void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
   };

#endif // TTGRAPHSCENE_H

