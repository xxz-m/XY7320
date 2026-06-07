#include <QGuiApplication>
#include <QIcon>
#include <QCoreApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "backend/FirmwareUploader.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setWindowIcon(QIcon(QStringLiteral(":/xy7320host/assets/images/icon.ico")));

    FirmwareUploader firmwareUploader;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("firmwareUploader"), &firmwareUploader);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule(QStringLiteral("XY7320Host"), QStringLiteral("Main"));

    return app.exec();
}
