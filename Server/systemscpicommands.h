#ifndef SYSTEMSCPICOMMANDS_H
#define SYSTEMSCPICOMMANDS_H

#include <QObject>
#include "commonscpicommands.h"

class SystemScpiCommands: public CommonScpiCommands
{
    Q_OBJECT
public:
    explicit SystemScpiCommands(QObject * parent = nullptr);

    // CommonScpiCommands interface
protected:
//    bool executeCommand(QString command, QString params) override;
    bool executeCommand(CmdToken token) override;
};

#endif // SYSTEMSCPICOMMANDS_H
