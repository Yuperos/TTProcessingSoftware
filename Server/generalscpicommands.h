#ifndef GENERALSCPICOMMANDS_H
#define GENERALSCPICOMMANDS_H

#include <QObject>
#include "commonscpicommands.h"

class GeneralScpiCommands: public CommonScpiCommands
{
    Q_OBJECT
public:
    explicit GeneralScpiCommands(QObject * parent = nullptr);

    // CommonScpiCommands interface
protected:
    bool executeCommand(CmdToken token) override;
//    bool executeCommand(QString command, QString params) override;
};

#endif // GENERALSCPICOMMANDS_H
