#ifndef CABSTRACTCOMMANDPARSER_H
#define CABSTRACTCOMMANDPARSER_H

#include <QObject>
#include "CmdTypes.h"
#include "ccommandexecutor.h"

class CmdToken{
public:
   CmdType mCmdType;
   uint8_t mDevAddress;
   QByteArray mCommand;
   QByteArray mParams;
   
   CmdToken();
   CmdToken(CmdType cmdType, uint8_t devAddress, QByteArray cmd, QByteArray params);
   CmdToken operator =(const CmdToken &other);

   bool isEmpty();
   void clear();

   QString params();
   QString command();
   };

class cAbstractCommandParser : public QObject
{
    Q_OBJECT
public:
    explicit cAbstractCommandParser(QObject *parent = nullptr);
//    bool parse(const QByteArray &ba); //INFO: Заменена новой функцией, после тестов будет удалена.
    CmdToken parse(const QByteArray &ba);
    virtual bool executeCommand(CmdToken token);
    virtual void initializeParser(){}

    cCommandExecutor *executor() const;
    void setExecutor(cCommandExecutor *executor);

protected:
    virtual bool isCorrectCommand(const QByteArray &ba);
    virtual CmdToken parseCommand(const QByteArray &ba);

private:
    cCommandExecutor * mExecutor;

};

#endif // CABSTRACTCOMMANDPARSER_H
