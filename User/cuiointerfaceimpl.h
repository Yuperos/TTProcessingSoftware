#ifndef CUIOINTERFACEIMPL_H
#define CUIOINTERFACEIMPL_H

#include <QObject>
#include "cuiointerface.h"

//TODO: Описать все поля класса cuIOInterfaceImpl
/**
 * @brief The cuIOInterfaceImpl class
 */
class cuIOInterfaceImpl : public cuIOInterface
{
    Q_OBJECT
public:
    /**
     * @brief cuIOInterfaceImpl
     * @param parent
     */
    explicit cuIOInterfaceImpl(QObject *parent = nullptr);
    /**
     * @brief initialize
     * @return
     */
    virtual bool initialize();
    /**
     * @brief sendMsg
     * @param address
     * @param command
     * @param dataLength
     * @param data
     * @return
     */
    virtual bool sendMsg(quint8 address, quint8 command, quint8 dataLength, quint8* data);

protected:
    /**
     * @brief pInitialize
     * @return
     */
    virtual bool pInitialize() = 0;

    /**
     * @brief pSendMsg
     * @param address
     * @param command
     * @param dataLength
     * @param data
     * @return
     */
    virtual bool pSendMsg(quint8 address, quint8 command, quint8 dataLength, quint8* data) = 0;

};

#endif // CUIOINTERFACEIMPL_H
