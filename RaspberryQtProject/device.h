#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QVariantMap>
#include "deviceinfo.h"

class Device : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariant devicesList READ getDevices NOTIFY devicesUpdated)
    Q_PROPERTY(QString update READ getUpdate WRITE setUpdate NOTIFY updateChanged)

public:
    explicit Device(QObject *parent = nullptr);
    QVariant getDevices();
    QString getUpdate();

    Q_INVOKABLE void startScan();
    Q_INVOKABLE void startConnect(QString addr);
    Q_INVOKABLE void disconnectBle();
    Q_INVOKABLE bool isConnected();
    Q_INVOKABLE bool isConnecting();

signals:
    void dataRx(QByteArray data);
    void bleError(QString info);
    void connected();
    void disconnected();
    void devicesUpdated();
    void updateChanged();

public slots:
    void writeData(QByteArray data);
    void writeData(QString strData);
    void clearDeviceList();

private slots:
    void addDevice(const QBluetoothDeviceInfo &dev);
    void scanFinished();
    void deviceScanError(QBluetoothDeviceDiscoveryAgent::Error e);
    void serviceDiscovered(const QBluetoothUuid &);
    void serviceScanDone();
    void controllerError(QLowEnergyController::Error);
    void deviceConnected();
    void deviceDisconnected();
    void serviceStateChanged(QLowEnergyService::ServiceState s);
    void updateData(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void confirmedDescriptorWrite(const QLowEnergyDescriptor &d, const QByteArray &value);
    void connectionUpdated(const QLowEnergyConnectionParameters &newParameters);

private:
    void setUpdate(QString message);
    QBluetoothDeviceDiscoveryAgent *m_DeviceDiscoveryAgent;
    QLowEnergyController *m_Control;
    QLowEnergyService *m_Service;
    QLowEnergyDescriptor m_NotificationDescTx;
    QList<QObject*> m_devices;
    bool m_UartServiceFound;
    bool m_ConnectDone;
    QString m_ServiceUuid;
    QString m_RxUuid;
    QString m_TxUuid;
    QString m_messageStatus;

};

#endif // DEVICE_H
