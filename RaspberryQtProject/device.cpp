#include "device.h"
#include <QDebug>
#include <QLowEnergyConnectionParameters>

Device::Device(QObject *parent) : QObject(parent)
{
    m_Control = nullptr;
    m_Service = nullptr;
    m_UartServiceFound = false;
    m_ConnectDone = false;

    m_ServiceUuid = "6e400001-b5a3-f393-e0a9-e50e24dcca9e";
    m_RxUuid = "6e400002-b5a3-f393-e0a9-e50e24dcca9e";
    m_TxUuid = "6e400003-b5a3-f393-e0a9-e50e24dcca9e";

    m_DeviceDiscoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    m_DeviceDiscoveryAgent->setLowEnergyDiscoveryTimeout(5000);

    connect(m_DeviceDiscoveryAgent, SIGNAL(deviceDiscovered(const QBluetoothDeviceInfo&)), this, SLOT(addDevice(const QBluetoothDeviceInfo&)));
    connect(m_DeviceDiscoveryAgent, SIGNAL(error(QBluetoothDeviceDiscoveryAgent::Error)), this, SLOT(deviceScanError(QBluetoothDeviceDiscoveryAgent::Error)));
    connect(m_DeviceDiscoveryAgent, SIGNAL(finished()), this, SLOT(scanFinished()));
}

void Device::startScan()
{
    m_devices.clear();

    m_DeviceDiscoveryAgent->start();

    setUpdate("Scanning...");
}

void Device::startConnect(QString addr)
{
    disconnectBle();

    m_UartServiceFound = false;
    m_ConnectDone = false;
    m_Control = new QLowEnergyController(QBluetoothAddress(addr));
    m_Control->setRemoteAddressType(QLowEnergyController::RandomAddress);

    connect(m_Control, SIGNAL(serviceDiscovered(QBluetoothUuid)), this, SLOT(serviceDiscovered(QBluetoothUuid)));
    connect(m_Control, SIGNAL(discoveryFinished()), this, SLOT(serviceScanDone()));
    connect(m_Control, SIGNAL(error(QLowEnergyController::Error)), this, SLOT(controllerError(QLowEnergyController::Error)));
    connect(m_Control, SIGNAL(connected()), this, SLOT(deviceConnected()));
    connect(m_Control, SIGNAL(disconnected()), this, SLOT(deviceDisconnected()));
    connect(m_Control, SIGNAL(connectionUpdated(QLowEnergyConnectionParameters)), this, SLOT(connectionUpdated(QLowEnergyConnectionParameters)));

    m_Control->connectToDevice();
}

void Device::disconnectBle()
{
    if (m_Service)
    {
        m_Service->deleteLater();
        m_Service = nullptr;
    }

    if (m_Control)
    {
        m_Control->disconnectFromDevice();
        m_Control->deleteLater();
        m_Control = nullptr;
    }
}

void Device::clearDeviceList()
{
    m_devices.clear();
    emit devicesUpdated();
}

bool Device::isConnected()
{
    return m_Control != nullptr && m_ConnectDone;
}

bool Device::isConnecting()
{
    return m_Control && !m_ConnectDone;
}

void Device::writeData(QByteArray data)
{
    if (isConnected())
    {
        const QLowEnergyCharacteristic  rxChar = m_Service->characteristic(QBluetoothUuid(QUuid(m_RxUuid)));
        if (rxChar.isValid())
        {
            while(data.size() > 20)
            {
                m_Service->writeCharacteristic(rxChar, data.mid(0, 20), QLowEnergyService::WriteWithResponse);
                data.remove(0, 20);
            }
            m_Service->writeCharacteristic(rxChar, data, QLowEnergyService::WriteWithResponse);
        }
    }
}

void Device::writeData(QString strData)
{
    if (isConnected())
    {
        const QLowEnergyCharacteristic  rxChar = m_Service->characteristic(QBluetoothUuid(QUuid(m_RxUuid)));
        if (rxChar.isValid())
        {
            QByteArray data = strData.toUtf8();

            m_Service->writeCharacteristic(rxChar, data, QLowEnergyService::WriteWithResponse);
        }
    }
}

void Device::addDevice(const QBluetoothDeviceInfo &dev)
{
    if (dev.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration)
    {
        DeviceInfo *d = new DeviceInfo(dev);
        m_devices.append(d);
    }
}

QVariant Device::getDevices()
{
    return QVariant::fromValue(m_devices);
}

QString Device::getUpdate()
{
    return m_messageStatus;
}

void Device::setUpdate(QString message)
{
    m_messageStatus = message;
    emit updateChanged();
}

void Device::scanFinished()
{
    emit devicesUpdated();

    if (m_devices.isEmpty())
        setUpdate("No Low Energy devices found...");
    else
        setUpdate("Done! Scan Again!");
}

void Device::deviceScanError(QBluetoothDeviceDiscoveryAgent::Error e)
{
    qWarning() << "BLE Scan error: " << e;
    m_devices.clear();
    emit devicesUpdated();
    setUpdate("BLE Scan error!");
}

void Device::serviceDiscovered(const QBluetoothUuid &gatt)
{
    if (gatt==QBluetoothUuid(QUuid(m_ServiceUuid)))
    {
        qDebug() << "BLE UART service found!";
        m_UartServiceFound = true;
    }
}

void Device::serviceScanDone()
{
    if (m_Service)
    {
        m_Service->deleteLater();
        m_Service = nullptr;
    }

    if (m_UartServiceFound)
    {
        qDebug() << "Connecting to BLE UART service";
        m_Service = m_Control->createServiceObject(QBluetoothUuid(QUuid(m_ServiceUuid)), this);

        connect(m_Service, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(serviceStateChanged(QLowEnergyService::ServiceState)));
        connect(m_Service, SIGNAL(characteristicChanged(QLowEnergyCharacteristic,QByteArray)), this, SLOT(updateData(QLowEnergyCharacteristic,QByteArray)));
        connect(m_Service, SIGNAL(descriptorWritten(QLowEnergyDescriptor,QByteArray)), this, SLOT(confirmedDescriptorWrite(QLowEnergyDescriptor,QByteArray)));

        m_Service->discoverDetails();
    } else {
        qWarning() << "BLE UART service not found";
        disconnectBle();
    }
}

void Device::controllerError(QLowEnergyController::Error e)
{
    qWarning() << "BLE error:" << e;
    disconnectBle();
    emit bleError(tr("BLE error: "));
}

void Device::deviceConnected()
{
    qDebug() << "BLE device connected";
    m_Control->discoverServices();
}

void Device::deviceDisconnected()
{
    qDebug() << "BLE service disconnected";
    emit disconnected();
    disconnectBle();
}

void Device::serviceStateChanged(QLowEnergyService::ServiceState s)
{
    // A descriptor can only be written if the service is in the ServiceDiscovered state
    switch (s) {
    case QLowEnergyService::ServiceDiscovered: {
        //looking for the TX characteristic
        const QLowEnergyCharacteristic txChar = m_Service->characteristic(
                    QBluetoothUuid(QUuid(m_TxUuid)));

        if (!txChar.isValid()){
            qDebug() << "BLE Tx characteristic not found";
            break;
        }

        //looking for the RX characteristic
        const QLowEnergyCharacteristic  rxChar = m_Service->characteristic(QBluetoothUuid(QUuid(m_RxUuid)));

        if (!rxChar.isValid()) {
            qDebug() << "BLE Rx characteristic not found";
            break;
        }

        // Bluetooth LE spec Where a characteristic can be notified, a Client Characteristic Configuration descriptor
        // shall be included in that characteristic as required by the Bluetooth Core Specification
        // Tx notify is enabled
        m_NotificationDescTx = txChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);

        if (m_NotificationDescTx.isValid()) {
            // enable notification
            m_Service->writeDescriptor(m_NotificationDescTx, QByteArray::fromHex("0100"));
        }

        break;
    }

    default:
        break;
    }
}

void Device::updateData(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    if (c.uuid() == QBluetoothUuid(QUuid(m_TxUuid)))
    {
        emit dataRx(value);
    }
}

void Device::confirmedDescriptorWrite(const QLowEnergyDescriptor &d, const QByteArray &value)
{
    if (d.isValid() && d == m_NotificationDescTx && value == QByteArray("0000"))
    {
        disconnectBle();
    } else {
        m_ConnectDone = true;
        emit connected();
    }
}

void Device::connectionUpdated(const QLowEnergyConnectionParameters &newParameters)
{
    (void)newParameters;
    qDebug() << "BLE connection parameters updated";
}
