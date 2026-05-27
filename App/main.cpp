
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "autogen/environment.h"
#include "gui/GuiConnector.h"




int main(int argc, char *argv[]) {
    set_qt_environment();
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;

    GuiConnector guiConnector = GuiConnector(engine.parent());

    qmlRegisterType<GuiConnector>("libGui", 1, 0, "GuiConnectorClass");

    const QUrl url(mainQmlFile);
    QObject::connect(
                &engine, &QQmlApplicationEngine::objectCreated, &app,
                [url, &guiConnector](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
            QCoreApplication::exit(-1);
        }
                    // Only when connect init device
        guiConnector.init_device();
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
    QObject * rootObject  = engine.rootObjects().first();

    QObject::connect(&guiConnector,
                  SIGNAL(device_updated(QVariant)),rootObject ,
                  SLOT(deviceUpdated(QVariant)),
                        Qt::QueuedConnection);


    return QApplication::exec();
}
