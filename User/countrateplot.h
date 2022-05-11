#ifndef COUNTRATEPLOT_H
#define COUNTRATEPLOT_H

#include <QDialog>
#include <QStatusBar>
#include "graphstorage.h"

namespace Ui {
   class CountratePlot;
   }

class CountratePlot : public QDialog
   {
   Q_OBJECT

   bool isRecordStarted = false;
   QString savePath;
   QString filename;
   GraphStorage* storage;
   QColor currentColor;
   QStatusBar *sBar;

   double currentTimeMs;
   QList<QColor> colorMap;

   int timeRange = 50000;

public:
   explicit CountratePlot(GraphStorage* storagePtr,QWidget *parent = nullptr);
   ~CountratePlot();

   void setSaveFolder(QString path);
   bool createNewFile();
   void createColorMap();

public slots:
   void updateData();
   void setVisible(bool visible) override;
   void callOpenFolderDialog();
   void updatePathLinks();
   void updateTreshold(int value);
   void updateDataAccumulationProgress();
   void startStopRecordPressed(bool isRecordStarted);
   void closeFile();

   void plotWheelEvent(QWheelEvent* event);

signals:
   void visibilityChanged(bool);
   void savePathChanged();

private:
   Ui::CountratePlot *ui;

   };

#endif // COUNTRATEPLOT_H
