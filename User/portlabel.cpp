#include "portlabel.h"
#include <QDebug>
#include <QMouseEvent>

bool PortLabel::isSilenced = true;
int PortLabel::isHighlighted = 0;

void PortLabel::setPortNumber(int value)
   {
   portNumber = value;
   if (portNumber > -512)
      setText(QString::number(portNumber));
   else
      setText("?");
   setToolTip(QString::number(portNumber));
   }

int PortLabel::getPortNumber() const
   {
   return portNumber;
   }

PortLabel::PortLabel(QString nPortNumber, int portType, QWidget *parent):
   QLabel(parent)
   {
   highLightCode = portType;
   setPortNumber(nPortNumber.toInt());

   setMinimumSize(30,20);
   setMaximumSize(30,20);
   setAlignment(Qt::AlignCenter);
   setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
   setFrameStyle(QFrame::Box);
   }

void PortLabel::mousePressEvent(QMouseEvent *event)
   {
   QRect tempRect = geometry();
   tempRect.moveTo(0,0);
   if (tempRect.contains(event->pos())){
      if (event->button() == Qt::LeftButton){
         event->accept();
         qDebug() << "Portmouse Left";
         emit channelLeftClicked();
         }
      else if (event->button() == Qt::RightButton){
         event->accept();
         qDebug() << "Portmouse Right";
         emit channelRightClicked(portNumber);
         }
      }
   else
      QLabel::mousePressEvent(event);
   }

void PortLabel::paintEvent(QPaintEvent *event)
   {
   if (isHighlighted == highLightCode)
      setStyleSheet("QLabel { background-color : lightGreen; }");
   else
      setStyleSheet(QString());

   QLabel::paintEvent(event);
   }
