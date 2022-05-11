#include "cuiointerfaceimpl.h"

cuIOInterfaceImpl::cuIOInterfaceImpl(QObject *parent):
    cuIOInterface(parent)
{

}

bool cuIOInterfaceImpl::initialize()
{
    return pInitialize();
}

bool cuIOInterfaceImpl::sendMsg(quint8 address, quint8 command, quint8 dataLength, quint8 *data)
{
    return pSendMsg(address, command, dataLength, data);
}
