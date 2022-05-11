#ifndef CUIOINTERFACE_H
#define CUIOINTERFACE_H

#include <QObject>

//Общий интерфейс ко всем видам интерфейсов

//TODO: Описать все поля класса cuIOInterface
/**
 * @brief The cuIOInterface class
 */
class cuIOInterface : public QObject
{
    Q_OBJECT
public:

    /**
     * @brief cuIOInterface
     * @param parent
     */
    explicit cuIOInterface(QObject *parent = nullptr): QObject(parent){}

    /**
     * @brief initialize
     * @return
     */
    virtual bool initialize() = 0;

    /**
     * @brief sendMsg
     * @param address
     * @param command
     * @param dataLength
     * @param data
     * @return
     */
    virtual bool sendMsg(quint8 address, quint8 command, quint8 dataLength, quint8* data) = 0;
signals:
    /**
     * @brief msgReceived
     * @param address
     * @param command
     * @param dataLength
     * @param data
     */
    void msgReceived(quint8, quint8, quint8, quint8*);
};

#endif // CUIOINTERFACE_H
