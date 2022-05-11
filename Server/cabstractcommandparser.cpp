#include <QTextCodec>
#include "cabstractcommandparser.h"

cAbstractCommandParser::cAbstractCommandParser(QObject *parent)
    : QObject(parent)
{
    mExecutor = qobject_cast<cCommandExecutor*>(parent);
}

CmdToken cAbstractCommandParser::parse(const QByteArray &ba)
{
    if (isCorrectCommand(ba))
        return parseCommand(ba);
    return CmdToken();
}

bool cAbstractCommandParser::isCorrectCommand(const QByteArray &ba)
{
    Q_UNUSED(ba)
    return false;
}

CmdToken cAbstractCommandParser::parseCommand(const QByteArray &ba)
{
    Q_UNUSED(ba)
   return CmdToken();
   }

bool cAbstractCommandParser::executeCommand(CmdToken token)
   {
   Q_UNUSED(token)
   return false;
   }

cCommandExecutor *cAbstractCommandParser::executor() const
{
    return mExecutor;
}

void cAbstractCommandParser::setExecutor(cCommandExecutor *executor)
{
    mExecutor = executor;
   }

CmdToken::CmdToken():mCmdType(_CT_NA),mDevAddress(0xff),mCommand(QByteArray()),mParams(QByteArray())
   { }

CmdToken::CmdToken(CmdType cmdType, uint8_t devAddress, QByteArray cmd, QByteArray params):
   mCmdType(cmdType),
   mDevAddress(devAddress),
   mCommand(cmd),
   mParams(params)
   { }

CmdToken CmdToken::operator =(const CmdToken &other)
   {
   if (this == &other)
      return *this;
   mCmdType = other.mCmdType;
   mDevAddress = other.mDevAddress;
   mCommand.clear();
   mCommand.append(other.mCommand);
   mParams.clear();
   mParams.append(other.mParams);
   return *this;
   }

bool CmdToken::isEmpty()
   {
   return mCommand.isEmpty();
   }

void CmdToken::clear()
   {
   mCmdType = _CT_NA;
   mDevAddress = 0;
   mCommand.clear();
   mParams.clear();
   }

QString CmdToken::params()
   {
   return QString(mParams);
   }

QString CmdToken::command()
   {
   return QString(mCommand);
   }
