#include "ScenePlot.h"
#include "ui_ScenePlot.h"

ScenePlot::ScenePlot(QWidget *parent) :
   QDialog(parent),
   ui(new Ui::ScenePlot)
   {
   ui->setupUi(this);


   currentTimeMs = QDateTime(QDate(2022,1,1),QTime(0,0,0)).msecsTo(QDateTime::currentDateTime());

   createColorMap();
//   updatePathLinks();
   //   ui->progressBar->setMaximum(ui->SB_ValuesCountTreshold->value()*1000);
   ui->customPlot->xAxis->setVisible(false);
   ui->customPlot->xAxis->setTickLabels(true);
   ui->customPlot->yAxis->setVisible(false);
   ui->customPlot->yAxis->setTickLabels(true);

   ui->customPlot->yAxis->setRange(0, 1000);
   ui->customPlot->xAxis->setRange(-1000 , 1000);

   ui->customPlot->setBackground(QBrush(QColor("#FFFFFF")));
   ui->customPlot->xAxis->setLabel("t (s)");
   ui->customPlot->yAxis->setLabel("counts");

   ui->customPlot->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom);
   ui->customPlot->axisRect()->setupFullAxesBox(true);

   timer = new QTimer();

   connect(timer,&QTimer::timeout,this,&ScenePlot::updateData);
   timer->setInterval(100);
   timer->start();
   show();

   //   connect(ui->PB_SelectFolder,&QPushButton::clicked,this,&CountratePlot::callOpenFolderDialog);
   //   connect(this, &CountratePlot::savePathChanged, this, &CountratePlot::updatePathLinks);
   //   connect(ui->SB_ValuesCountTreshold,qOverload<int>(&QSpinBox::valueChanged), this, &CountratePlot::updateTreshold);
   //   connect(ui->PB_StartStopRec,&QPushButton::toggled,this,&CountratePlot::startStopRecordPressed);
   //   connect(ui->PB_NextFile,&QPushButton::clicked,this,&CountratePlot::closeFile);
   //   connect(storage,&GraphStorage::rowAppended,this,&CountratePlot::createNewFile, Qt::DirectConnection);
   //   connect(storage,&GraphStorage::writedDataCountChanged, ui->progressBar, &QProgressBar::setValue);
   //   connect(ui->customPlot,&QCustomPlot::mouseWheel,this,&CountratePlot::plotWheelEvent);
   //   sBar = new QStatusBar(this);
   //   ui->gridLayout_2->addWidget(sBar);
   }

ScenePlot::~ScenePlot()
   {
   delete ui;
   }

void ScenePlot::createColorMap()
   {
   QColor tempColor = QColor::fromHsl(0,170,170);
   for(int i = 0; i< 81; i++){
      tempColor.setHsl((tempColor.hue()+40)%360,tempColor.hslSaturation(),tempColor.lightness());
      if (tempColor.hslHue() == 0)
         tempColor = tempColor.darker(100);
      colorMap.append(tempColor);
      }
   }

void ScenePlot::updateData()
   {
   ui->customPlot->clearGraphs();
   ui->customPlot->addGraph();
   for(int i = ui->customPlot->xAxis->range().lower; i < ui->customPlot->xAxis->range().upper; i++)
      ui->customPlot->graph(0)->addData(i,(1000./i)*(QRandomGenerator::global()->generate()%1000));
   ui->customPlot->yAxis->rescale(true);
   ui->customPlot->replot();
   }
