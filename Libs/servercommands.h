#ifndef SERVERCOMMANDS_H
#define SERVERCOMMANDS_H

#define SERVER_TCPIP_PORT                  9876

#define SERVER_ADDRESS              0xFF
#define MAX_DEVICE_TRY_COUNT        5

#define CMD_SERVER_GET_DEVICE_LIST  0
#define CMD_SERVER_SEARCH_DEVICES   1
#define CMD_SERVER_ADD_DEVICE       2
#define CMD_SERVER_SAVE_DEVICE_LIST 3

#include <QObject>

struct deviceInfo {
    quint8  devAddress;
    QString devType;
    QString devModVersion;
    QString devHwVersion;
    QString devFwVersion;
    QString devDescription;
};

#endif // SERVERCOMMANDS_H
