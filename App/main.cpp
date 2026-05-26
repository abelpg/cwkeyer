
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "autogen/environment.h"
#include "gui/GuiConnector.h"




int main(int argc, char *argv[]) {
    set_qt_environment();
    QApplication app(argc, argv);

    GuiConnector guiConnector;
    qmlRegisterType<GuiConnector>("libGui", 1, 0, "GuiConnectorClass");

    QQmlApplicationEngine engine;
    const QUrl url(mainQmlFile);
    QObject::connect(
                &engine, &QQmlApplicationEngine::objectCreated, &app,
                [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
            QCoreApplication::exit(-1);
        }

    }, Qt::QueuedConnection);




    engine.addImportPath(QCoreApplication::applicationDirPath() + "/qml");
    engine.addImportPath(":/");
    engine.load(url);

    qDebug() << "LOADED";

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }
    QQmlContext *context = engine.rootContext();
    context->setContextProperty("guiConnector", &guiConnector);

    // Here or engine.rootObjects().first()->children().first()
    QObject::connect(&guiConnector,
                  SIGNAL(device_updated(QVariant)), engine.rootObjects().first(),
                  SLOT(onDeviceUpdated(QVariant)),
                        Qt::QueuedConnection);

    return QApplication::exec();
}
