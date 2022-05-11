#include <QGraphicsItem>
#include <QGraphicsProxyWidget>
#include <QDebug>
#include <QPainter>

#include "ttgraphscene.h"
#include "channelblock.h"

void TTGraphScene::setLinksReconfigured(bool value)
   {
   linksReconfigured = value;
   }

TTGraphScene::TTGraphScene(QObject *parent) : QGraphicsScene(parent)
   {
   operationsMenu = new AddOperationMenu();
   sourceOutMenu = new SetSourceOutMenu();
   }

void TTGraphScene::setupLines()
   {
   inputs.clear();
   outputs.clear();
   if (!lines.isEmpty())
      for(int i = 0; i < lines.count(); i++){
         lines[i]->hide();
         delete lines[i];
         }
   lines.clear();

   getPortConfigurations();

//   qDebug() << inputs << outputs;

   for(auto out : outputs.uniqueKeys()){
      QPointF start = outputs.value(out);
      for(auto in : inputs.values(out)) {
         lines.append(addLine(QLineF(start,in)));
         lines.first()->setZValue(1);
         lines.first()->show();
         }
      }
   setLinksReconfigured(false);
   }

void TTGraphScene::updateLines()
   {

   if (lines.isEmpty()) return;

   getPortConfigurations();

   int t = 0;
   for(auto out : outputs.uniqueKeys()){
      QPointF start = outputs.value(out);
      for(auto in : inputs.values(out)) {
         if (lines.count() == t)
            lines.append(addLine(QLineF(start,in)));
         else
            lines[t]->setLine(QLineF(start,in));
         lines[t]->setZValue(1);
         lines[t]->show();
         t++;
         }
      }
   }

void TTGraphScene::getPortConfigurations()
   {
   inputs.clear();
   outputs.clear();
   for(auto a : items()){
      QGraphicsProxyWidget *tempWidget = qgraphicsitem_cast<QGraphicsProxyWidget*>(a);
      if(tempWidget != nullptr){
         ChannelBlock* tempBlock = qobject_cast<ChannelBlock*>(tempWidget->widget());
         if (tempBlock) {
            outputs.insert(tempBlock->getOutputs().first().first,tempWidget->scenePos()+tempBlock->getOutputs().first().second);
            for(auto in : tempBlock->getInputs()){
               inputs.insert(in.first,tempWidget->scenePos()+in.second);
               }
            }
         }
      }
   }

void TTGraphScene::drawForeground(QPainter *painter, const QRectF &rect)
   {
   if (linksReconfigured)
      setupLines();
   else
      updateLines();

   QGraphicsScene::drawForeground(painter,rect);
   }

void TTGraphScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
   {
   QGraphicsScene::mousePressEvent(event);
   if (event->button() == Qt::RightButton){
      if (event->isAccepted() == false)
         operationsMenu->popup(event->screenPos());
      }
   }
