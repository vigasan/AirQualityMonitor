//#include <QGuiApplication>
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "bridge.h"
#include "device.h"

int main(int argc, char *argv[])
{
    /*
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QGuiApplication app(argc, argv);

    */

    QApplication app(argc, argv);

    Device bleDevice;
    Bridge *appBridge = new Bridge();

    QObject::connect(&bleDevice, SIGNAL(dataRx(QByteArray)), appBridge, SLOT(dataReceived(QByteArray)));
    QObject::connect(&bleDevice, SIGNAL(connected()), appBridge, SLOT(deviceConnected()));
    QObject::connect(&bleDevice, SIGNAL(disconnected()), appBridge, SLOT(deviceDisconnected()));

    QQmlApplicationEngine engine;
    QQmlContext* ctx = engine.rootContext();
    ctx->setContextProperty("appBridge", appBridge);
    ctx->setContextProperty("device", &bleDevice);


    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
