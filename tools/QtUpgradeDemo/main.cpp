#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "firmwareuploader.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    FirmwareUploader uploader;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("firmwareUploader", &uploader);
    engine.loadFromModule("QtUpgradeDemo", "Main");

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
