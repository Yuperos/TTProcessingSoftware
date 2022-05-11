#ifndef SCENEPLOT_H
#define SCENEPLOT_H

#include <QDialog>

#include "qcustomplot.h"

namespace Ui {
   class ScenePlot;
   }

class ScenePlot : public QDialog
   {
   Q_OBJECT

   double currentTimeMs;
   QList<QColor> colorMap;
   QTimer *timer;

public:
   explicit ScenePlot(QWidget *parent = nullptr);
   ~ScenePlot();

   void createColorMap();

public slots:
   void updateData();

private:
   Ui::ScenePlot *ui;
   };

#endif // SCENEPLOT_H
