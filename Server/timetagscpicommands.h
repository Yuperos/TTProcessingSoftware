#ifndef TIMETAGSCPICOMMANDS_H
#define TIMETAGSCPICOMMANDS_H

#include <QObject>
#include "commonscpicommands.h"

class TimeTagScpiCommands: public CommonScpiCommands
{
    Q_OBJECT
public:
    explicit TimeTagScpiCommands(QObject * parent = nullptr);
    // CommonScpiCommands interface
protected:
//    bool executeCommand(QString command, QString params) override;
    bool executeCommand(CmdToken token) override;

private:
    processingAnswer processingCommand(quint8 address, QString command, QString params);
};

#endif // TIMETAGSCPICOMMANDS_H
