#include "bridge.h"
#include <QDebug>

Bridge::Bridge(QObject *parent) : QObject(parent)
{
}

void Bridge::deviceConnected()
{
    qDebug() << "CONNESSO!!!";
}

void Bridge::deviceDisconnected()
{
    qDebug() << "DISCONNESSO!!!";
    emit connectionChanged(DEVICE_DISCONNECTED);
}

void Bridge::dataReceived(QByteArray receivedData)
{
    if(receivedData.count() == 14)
    {
        int temperature = int((receivedData[0] << 8) | (receivedData[1]));
        int humidity = int((receivedData[2] << 8) | (receivedData[3]));
        int pm2_5 = int((receivedData[4] << 8) | (receivedData[5]));
        int formaldeyde = int((receivedData[6] << 8) | (receivedData[7]));
        int ozone = int((receivedData[8] << 8) | (receivedData[9]));
        unsigned int co2 = (unsigned int)((receivedData[10] << 24) | (receivedData[11] << 16) | (receivedData[12] << 8) | (receivedData[13]));

        emit sensorData(temperature, humidity, pm2_5, formaldeyde, ozone, co2);
    }
}

