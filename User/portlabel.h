#ifndef PORTLABEL_H
#define PORTLABEL_H

#include <QLabel>

class PortLabel : public QLabel
   {
   Q_OBJECT

   int highLightCode = 0;
   int portNumber = 0;

public:
   static bool isSilenced;
   static int isHighlighted;

   explicit PortLabel(QString nPortNumber, int portType, QWidget* parent);

   void setPortNumber(int value);
   int getPortNumber() const;

signals:
   void channelLeftClicked();
   void channelRightClicked(int);


   // QWidget interface
protected:
   void mousePressEvent(QMouseEvent *event) override;
   void paintEvent(QPaintEvent *event) override;
   };

#endif // PORTLABEL_H
