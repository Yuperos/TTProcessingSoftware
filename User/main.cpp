#include <QApplication>
#include <QCommandLineParser>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QElapsedTimer>
#include <QStyle>
#include <QDesktopWidget>
#include <QDebug>

#include "channelblock.h"
#include "proxyblockwidget.h"
#include "mainwindow.h"

#include "swabiancontrol.h"

int main(int argc, char *argv[])
   {
   QApplication a(argc, argv);

//   QCoreApplication::addLibraryPath("c:/Program Files/Swabian Instruments/TimeTagger2_7_6/driver/x86");

   MainWindow w;
   w.show();

   return a.exec();
   }
