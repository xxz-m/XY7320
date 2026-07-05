#include <QGuiApplication>
#include <QIcon>
#include <QCoreApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "backend/FirmwareUploader.h"
#include "backend/SerialPortManager.h"
#include "backend/SerialDebug.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setWindowIcon(QIcon(QStringLiteral(":/xy7320host/assets/images/icon.ico")));

    SerialPortManager serialPortManager;
    FirmwareUploader firmwareUploader(&serialPortManager);
    SerialDebug serialDebug(&serialPortManager);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("firmwareUploader"), &firmwareUploader);
    engine.rootContext()->setContextProperty(QStringLiteral("serialDebug"), &serialDebug);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule(QStringLiteral("XY7320Host"), QStringLiteral("Main"));

    return app.exec();
}
