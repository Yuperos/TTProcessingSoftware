#ifndef SCREENMENU_H
#define SCREENMENU_H

#include <QMenu>


class AddOperationMenu : public QMenu
   {
   Q_OBJECT

public:
   explicit AddOperationMenu(QWidget *parent = nullptr);
   ~AddOperationMenu();
   };

class SetSourceOutMenu : public QMenu
   {
   Q_OBJECT

public:
   void update();
   explicit SetSourceOutMenu(QWidget *parent = nullptr);
   ~SetSourceOutMenu();
   };

#endif // SCREENMENU_H
