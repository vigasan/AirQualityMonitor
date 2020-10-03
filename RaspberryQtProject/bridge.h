#ifndef BRIDGE_H
#define BRIDGE_H

#include <QObject>

#define DEVICE_DISCONNECTED 0
#define DEVICE_CONNECTED    1

class Bridge : public QObject
{
    Q_OBJECT

public:
    explicit Bridge(QObject *parent = nullptr);

signals:
    void sensorData(int temp, int hum, int pm2_5, int formaldeyde, int ozone, unsigned int co2);
    void connectionChanged(int value);

public slots:
    void dataReceived(QByteArray receivedData);
    void deviceConnected();
    void deviceDisconnected();
};

#endif // BRIDGE_H
