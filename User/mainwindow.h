#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QTimer>
#include <QStyle>

#include "proxyblockwidget.h"
#include "ttgraphscene.h"
#include "graphstorage.h"
#include "cutcpsocketiointerface.h"
#include "countrateplot.h"
#include "channelblock.h"
#include "qCustomLib.h"
#include "BlockHub.h"

enum ServerCommunicationState{
   SCS_Idle,
   SCS_ServerConnection,
   SCS_TaggerConnection,
   SCS_SyncBlocks,
   SCS_SyncCountRateParams,
   SCS_SyncDumpChannels,
   SCS_GetCountrate,
   };


namespace Ui {
   class MainWindow;
   }

class MainWindow : public QMainWindow
   {
   Q_OBJECT
   TTGraphScene* scene;
   QMap<int,TTOpExchangeWrapper* > ttOperations;
   QMap<int,ChannelBlock *> blocks;

   bool isServerConnected = false;
   bool isTaggerConnected = false;

   bool needToUpdateDumpChannels = false;

   QPair<int,int> clickedPort;
   int procedureInterval = 2000;

   QTimer* connectionWatchdog;

   QList<TTOpExchangeWrapper*> blocksToSync;
   QList<int> channelsToDump;
   ServerCommunicationState commState = SCS_ServerConnection;
   cuTcpSocketIOInterface* tcpInterface;
//   TTGraphScene* scene;

//   QMap<int,TTOpExchangeWrapper* > ttOperations;
//   QMap<int,ChannelBlock *> blocks;

   GraphStorage storage;
   CountratePlot* plot;

public:
   explicit MainWindow(QWidget *parent = nullptr);
   ~MainWindow();

   void initOperationBlocks();
   void updateOperationBlocks();

   void getTTOperations();
   void layoutBlocks();
   void addBlock(TTOpExchangeWrapper *ttStruct, QRectF rect = QRectF());
   void removeBlock(int id);

   bool connectToServer();
   bool connectTagger();
   bool syncBlock(TTOpExchangeWrapper* wrapper);
   int syncCountRateParams();
   int syncDumpChannels();
   int getCountrateChCount();
   int getDumpChannelsCount();
   QMap<int, double> getCountrate(int &ts);


   void moveBlock(int prevIdx, int newIdx);
   void checkAndResolveMoveConflicts(int prevIdx, int newIdx);
   void findBlocksToSync();
   bool isSiblingsConfirmed(TTOpExchangeWrapper* node);

   ServerCommunicationState getCommState() const;
   void setCommState(const ServerCommunicationState &value);

public slots:
   void checkConnections();
   void comunicationProcedure();
   void serverDisconnected();

   bool isOutput(QPair<int,int> pair);
   void recievePortClick(int id, int portIdx);
   int getPortNumber(QPair<int,int> pair);
   void recieveBlockClick(int id);
   void recieveDumpClick(bool isChecked);
   void resetHighlight();
   void recieveSyncStatusChanged(SynchronizationStage stage);
   void recieveIsCountrateNeededChanged(bool isNeeded);
   void processMenuAction(QAction *act);
   void processSourceMenuAction(QAction *act);

private:
   void updateStatus();
   void save();
   void load();

   Ui::MainWindow *ui;

protected:
   void keyPressEvent(QKeyEvent *event) override;
   };

class GraphicsViewZoom : public QObject {
  Q_OBJECT
public:
  GraphicsViewZoom(QGraphicsView* view);
  void gentleZoom(double factor);
  void setModifiers(Qt::KeyboardModifiers modifiers);
  void setZoomFactorBase(double value);

private:
  QGraphicsView* _view;
  Qt::KeyboardModifiers _modifiers;
  double _zoom_factor_base;
  QPointF target_scene_pos, target_viewport_pos;
  bool eventFilter(QObject* object, QEvent* event);

signals:
  void zoomed();
};

#endif // MAINWINDOW_H

