#include <QFileDialog>
#include <QDir>

#include "countrateplot.h"
#include "ui_countrateplot.h"


CountratePlot::CountratePlot(GraphStorage *storagePtr, QWidget *parent) :
   storage(storagePtr),
   QDialog(parent),
   ui(new Ui::CountratePlot)
   {
   ui->setupUi(this);

   currentTimeMs = QDateTime(QDate(2022,1,1),QTime(0,0,0)).msecsTo(QDateTime::currentDateTime());

   createColorMap();
   updatePathLinks();
   ui->progressBar->setMaximum(ui->SB_ValuesCountTreshold->value()*1000);
   ui->customPlot->xAxis->setVisible(false);
   ui->customPlot->xAxis->setTickLabels(true);
   ui->customPlot->yAxis->setVisible(false);
   ui->customPlot->yAxis->setTickLabels(true);

   ui->customPlot->yAxis->setRange(-10000 , 10000);

   ui->customPlot->setBackground(QBrush(QColor("#FFFFFF")));
   ui->customPlot->xAxis->setLabel("t (s)");
   ui->customPlot->yAxis->setLabel("counts");

   ui->customPlot->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom);
   ui->customPlot->axisRect()->setupFullAxesBox(true);

   connect(ui->PB_SelectFolder,&QPushButton::clicked,this,&CountratePlot::callOpenFolderDialog);
   connect(this, &CountratePlot::savePathChanged, this, &CountratePlot::updatePathLinks);
   connect(ui->SB_ValuesCountTreshold,qOverload<int>(&QSpinBox::valueChanged), this, &CountratePlot::updateTreshold);
   connect(ui->PB_StartStopRec,&QPushButton::toggled,this,&CountratePlot::startStopRecordPressed);
   connect(ui->PB_NextFile,&QPushButton::clicked,this,&CountratePlot::closeFile);
   connect(storage,&GraphStorage::rowAppended,this,&CountratePlot::createNewFile, Qt::DirectConnection);
   connect(storage,&GraphStorage::writedDataCountChanged, ui->progressBar, &QProgressBar::setValue);
   connect(ui->customPlot,&QCustomPlot::mouseWheel,this,&CountratePlot::plotWheelEvent);
   sBar = new QStatusBar(this);
   ui->gridLayout_2->addWidget(sBar);


   //   sensitivity = 0.5f + 50.0/4.0; // mid range
   }

CountratePlot::~CountratePlot()
   {
   delete ui;
   }

void CountratePlot::updateData()
   {
   ui->customPlot->clearGraphs();
   if (storage->dataRowsCount() > ui->customPlot->graphCount()){
      for(int i = ui->customPlot->graphCount(); i< storage->dataRowsCount() && i < colorMap.count(); i++){
         QPen pen;
         pen.setStyle(Qt::SolidLine);
         pen.setWidth(2);
         pen.setColor(colorMap.at(i));
         ui->customPlot->addGraph();
         ui->customPlot->graph(i)->setPen(pen);
         }
      }

   int dataCount = storage->countInRange(timeRange);

   for(int rowIdx = 0 ; rowIdx < storage->dataRowsCount(); rowIdx++){
      auto row = storage->getDataRow(rowIdx);
//      qDebug() << row.first();
      for(int i = 0; i < row.count() && i < dataCount; i++ ){
         ui->customPlot->graph(rowIdx)->addData((row.at(i).x()-currentTimeMs)/1000.,row.at(i).y());
         }
      }
//   qDebug() << currentTimeMs << storage->getLastTag();

//   double start = (stop-timeRange > 0)?(stop-timeRange/1000.)+2.5:2.5;

   updateDataAccumulationProgress();

   plotWheelEvent(nullptr);

   ui->customPlot->yAxis->rescale(true);
//   ui->customPlot->yAxis->
   ui->customPlot->replot();
   }

void CountratePlot::setVisible(bool visible)
   {
   QDialog::setVisible(visible);
   emit visibilityChanged(visible);
   }

void CountratePlot::setSaveFolder(QString path)
   {

   if (!path.isEmpty()){
      QDir dir(path);
      if (dir.exists()){
         savePath = path;
         emit savePathChanged();
         }
      }
   }

bool CountratePlot::createNewFile()
   {
   if (savePath.isEmpty())
      return false;

   filename = QString("TTPlotData_%1.csv").arg(QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss"));
   QString fullPath = QString("%1/%2").arg(savePath).arg(filename);
   QFile outFile(fullPath);

   if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)){
      sBar->showMessage(QString("ERROR creating file: ").arg(fullPath), 5000);
      return false;
      }
   outFile.close();
   sBar->showMessage(QString("File created: %1").arg(fullPath), 3000);
   storage->setFile(fullPath);
   return true;
   }

void CountratePlot::createColorMap()
   {
   QColor tempColor = QColor::fromHsl(0,170,170);
   for(int i = 0; i< 81; i++){
      tempColor.setHsl((tempColor.hue()+40)%360,tempColor.hslSaturation(),tempColor.lightness());
      if (tempColor.hslHue() == 0)
         tempColor = tempColor.darker(100);
      colorMap.append(tempColor);
      }
   }


void CountratePlot::callOpenFolderDialog()
   {
   setSaveFolder(QFileDialog::getExistingDirectory(this, tr("Open Directory"), savePath, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));
   }

void CountratePlot::updatePathLinks()
   {
   QString tittle("Countrate Plot");
   if (!savePath.isEmpty()){
      tittle.append(QString(". Save to: %1").arg(savePath));
      ui->PB_StartStopRec->setEnabled(true);
      ui->PB_NextFile->setEnabled(true);
      }
   else{
      ui->PB_StartStopRec->setEnabled(false);
      ui->PB_NextFile->setEnabled(false);
      }
   setWindowTitle(tittle);
   }

void CountratePlot::updateTreshold(int value)
   {
   if (ui->progressBar->value()*1000 > value){
      closeFile();
      updateDataAccumulationProgress();
      }
   ui->progressBar->setMaximum(value*1000);
   }

void CountratePlot::updateDataAccumulationProgress()
   {
//      storage->lock.lockForRead();
//   ui->progressBar->setValue(storage->getValuesCount());
//      storage->lock.unlock();
   }

void CountratePlot::startStopRecordPressed(bool isStart)
   {
   if (isStart != isRecordStarted){
      isRecordStarted = isStart;
      if (isStart && createNewFile()){
         ui->PB_StartStopRec->setText("Stop");
         }
      else
         {
         ui->PB_StartStopRec->setText("Start");
         }
      }
   }

void CountratePlot::closeFile()
   {
   createNewFile();
   }

void CountratePlot::plotWheelEvent(QWheelEvent *event)
   {
   double stop = (storage->getLastTag()-currentTimeMs)/1000.-1;
   if (ui->customPlot->xAxis->range().lower < 0)
      ui->customPlot->xAxis->setRangeLower(-10);
   ui->customPlot->xAxis->setRangeUpper(stop);
   ui->customPlot->yAxis->setRangeLower(-10);
   }
