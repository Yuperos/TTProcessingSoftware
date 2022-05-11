#include "proxyblockwidget.h"
#include <QWidget>

BlockProxyWidget::BlockProxyWidget(QGraphicsItem *parent, Qt::WindowFlags wFlags):
   QGraphicsProxyWidget(parent, wFlags)
   {
   }

void BlockProxyWidget::closeEvent(QCloseEvent *event)
   {
   emit closePressed();
   QGraphicsProxyWidget::closeEvent(event);
   }
