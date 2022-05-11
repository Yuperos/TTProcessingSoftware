#ifndef COMMONSCPICOMMANDS_H
#define COMMONSCPICOMMANDS_H

#include <QObject>
#include "cabstractcommandparser.h"

class CommonScpiCommands: public cAbstractCommandParser
{
    Q_OBJECT
public:
    explicit CommonScpiCommands(QObject * parent = nullptr);

    enum processingAnswer{
        PA_Ok,
        PA_TimeOut,
        PA_ErrorData,
        PA_WithoutAnswer,
        PA_None
    };

    QPair<float,float> pairFromJsonString(QString str, bool *ok);
    void sendPairFloat(QPair<float,float> data);

    // cAbstractCommandParser interface
protected:
    CmdType extractCmdType(QString str);
    uint8_t extractDevNumber(QString str);
    bool isCorrectCommand(const QByteArray &ba);
    CmdToken parseCommand(const QByteArray &ba);
    bool executeCommand(CmdToken token) override;
   };

#endif // COMMONSCPICOMMANDS_H
