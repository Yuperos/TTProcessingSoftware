#include "screenmenu.h"
#include "swabiancontrol.h"
#include <QSet>

AddOperationMenu::AddOperationMenu(QWidget *parent) :
   QMenu(parent)
   {
   for(int i = 0; i < (int)_TTOP_count; i++)
      addAction(TTOpExchangeStruct::toString((TTOperationType)i));
   }

AddOperationMenu::~AddOperationMenu()
   {

   }

void SetSourceOutMenu::update()
   {
   clear();
   auto list = TTOpExchangeStruct::avaliableSourceChannels.toList();
   std::sort(list.begin(),list.end());
   for(auto a : list){
      addAction(TTOpExchangeStruct::ChNumberToString(a));
      addAction(TTOpExchangeStruct::ChNumberToString(-a));
      }
   }

SetSourceOutMenu::SetSourceOutMenu(QWidget *parent) :
   QMenu(parent)
   {
   update();
   }

SetSourceOutMenu::~SetSourceOutMenu()
   {

   }
