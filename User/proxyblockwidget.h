#ifndef BLOCKPROXYWIDGET_H
#define BLOCKPROXYWIDGET_H

#include <QGraphicsProxyWidget>

class BlockProxyWidget : public QGraphicsProxyWidget
   {
   Q_OBJECT

public:
   explicit BlockProxyWidget(QGraphicsItem *parent = nullptr, Qt::WindowFlags wFlags = nullptr);

signals:
   void closePressed();

protected:
   void closeEvent(QCloseEvent *event) override;
   };

#endif // BLOCKPROXYWIDGET_H
