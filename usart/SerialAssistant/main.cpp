/**
 * @file main.cpp
 * @brief SerialAssistant application entry point.
 */

#include "backend/console/console_controller.h"
#include "backend/console/transmit_controller.h"
#include "backend/io/serial_port_controller.h"
#include "backend/settings/settings_manager.h"
#include "models/console_record_model.h"

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlError>
#include <QQuickStyle>
#include <QStandardPaths>
#include <QTextStream>

namespace {
void messageHandler(QtMsgType type, const QMessageLogContext& ctx, const QString& msg)
{
    static QFile file(QStandardPaths::writableLocation(QStandardPaths::TempLocation)
                      + QStringLiteral("/SerialAssistant.log"));
    static bool opened = false;
    if (!opened) {
        file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
        opened = true;
    }
    QTextStream ts(&file);
    const char* level = "INFO";
    switch (type) {
    case QtDebugMsg:    level = "DEBUG"; break;
    case QtInfoMsg:     level = "INFO";  break;
    case QtWarningMsg:  level = "WARN";  break;
    case QtCriticalMsg: level = "CRIT";  break;
    case QtFatalMsg:    level = "FATAL"; break;
    }
    ts << QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
       << " [" << level << "] " << msg;
    if (ctx.file) ts << "  (" << ctx.file << ":" << ctx.line << ")";
    ts << "\n";
    ts.flush();
    fprintf(stderr, "%s\n", msg.toLocal8Bit().constData());
    fflush(stderr);
}
}  // namespace

int main(int argc, char* argv[])
{
    qInstallMessageHandler(messageHandler);
    QQuickStyle::setStyle(QStringLiteral("Basic"));

    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("XYKJ"));
    QCoreApplication::setApplicationName(QStringLiteral("SerialAssistant"));
    app.setWindowIcon(QIcon(QStringLiteral(":/qt/qml/SerialAssistant/qml/assets/icons/app_icon.png")));

    SettingsManager settingsManager;
    SerialPortController serialPortController(&settingsManager);
    ConsoleRecordModel consoleRecordModel;
    ConsoleController consoleController(&serialPortController, &consoleRecordModel, &settingsManager);
    TransmitController transmitController(&serialPortController, &consoleRecordModel, &settingsManager);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("settingsManager"), &settingsManager);
    engine.rootContext()->setContextProperty(QStringLiteral("serialController"), &serialPortController);
    engine.rootContext()->setContextProperty(QStringLiteral("consoleController"), &consoleController);
    engine.rootContext()->setContextProperty(QStringLiteral("transmitController"), &transmitController);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        [](const QUrl& url) {
            qCritical() << "QML object creation failed for" << url.toString();
            QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::warnings,
        &app,
        [](const QList<QQmlError>& warnings) {
            for (const QQmlError& warning : warnings)
                qWarning().noquote() << warning.toString();
        });

    engine.loadFromModule(QStringLiteral("SerialAssistant"), QStringLiteral("Main"));
    if (engine.rootObjects().isEmpty()) {
        qCritical() << "No QML root objects were created.";
        return -1;
    }
    return app.exec();
}

