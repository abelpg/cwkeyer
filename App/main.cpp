
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "autogen/environment.h"
#include "gui/GuiConnector.h"
#include <windows.h>
#include "utils/Logger.h"

LogLevel loglevel = L_INFO;

int main(int argc, char *argv[]) {
    set_qt_environment();

    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

    QApplication app(argc, argv);
    QQmlApplicationEngine engine;
    QQmlContext *context = engine.rootContext();
    GuiConnector guiConnector = GuiConnector(&app, context->parent());
    context->setContextProperty("guiConnector", &guiConnector);

    const QUrl url(mainQmlFile);
    QObject::connect(
                &engine, &QQmlApplicationEngine::objectCreated, &app,
                [url, &guiConnector](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
            QCoreApplication::exit(-1);
        }
        // Only when connect init device
        guiConnector.initDevice();
    }, Qt::QueuedConnection);

    engine.addImportPath(QCoreApplication::applicationDirPath() + "/qml");
    engine.addImportPath(":/");
    engine.load(url);

    log(L_DEBUG) << "LOADED";

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    QObject *rootObject = engine.rootObjects().first();

    // When device connected
    QObject::connect(&guiConnector,
                     SIGNAL(deviceUpdated(QVariant)), rootObject,
                     SLOT(deviceUpdated(QVariant)),
                     Qt::QueuedConnection);

    // Text qso
    QObject::connect(&guiConnector,
                     SIGNAL(textCwDecoderUpdated(QVariant)), rootObject,
                     SLOT(textCwDecoderUpdated(QVariant)),
                     Qt::QueuedConnection);

    QObject::connect(&app, &QCoreApplication::aboutToQuit, &guiConnector, &GuiConnector::quit);

    return QApplication::exec();
}
