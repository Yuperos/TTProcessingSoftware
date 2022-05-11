#ifndef CHANNELBLOCK_H
#define CHANNELBLOCK_H

#include <QWidget>
#include <QLabel>
#include <QSpinBox>
#include <QStyleFactory>
#include <QStyle>

#include "portlabel.h"
#include "swabiancontrol.h"

namespace Ui {
   class ChannelBlock;
   }

class ChannelBlock : public QWidget
   {
   Q_OBJECT

   TTOpExchangeWrapper *header;
   QList<PortLabel*> inputLabels;
   PortLabel *outputLabel = nullptr;

public:
   explicit ChannelBlock(TTOpExchangeWrapper *nHeader, QWidget *parent = nullptr);
   ~ChannelBlock();

   QList<QPair<QLabel*,QSpinBox*> > parametersHolder;
   QList<QLabel*> paramUnits;

   void setupInputs();
   void updateInputs();
   void setupOutputs();
   void updateOutputs();
   void clearLayout(QLayout * layout);
   void updateGuiParams();
   void updateParamList();
   void updateStatus();
   void setParamGUI(int idx, QString name = QString(), QString units = QString(), int min = 0, int max = 0);

   void updateTTOperation();

   QList<QPair<int, QPoint> > getInputs();
   QList<QPair<int, QPoint> > getOutputs();

   void setHeader(TTOpExchangeWrapper *value);

   static void setStyleHelper(QWidget *widget, QStyle *style);

public slots:
   void recieveCountrateClick(bool isChecked);
   void recieveDumpClick(bool isChecked);
   void recieveChannelClicked(int channel);
   void recieveParameterChanged(int idx, double value);
   void recieveSyncStatusChanged(SynchronizationStage stage);
   void recieveCountrateValue(double value);
   void dumpDisable(bool value);

signals:
   void portClicked(int, int);
   void blockClicked(int);
   void blockClosed(int);

private:
   Ui::ChannelBlock *ui;

protected:
   void mousePressEvent(QMouseEvent *event) override;
   };

#endif // CHANNELBLOCK_H
