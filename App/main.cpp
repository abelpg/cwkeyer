// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "autogen/environment.h"
#include "core/sound.h"
#include "usb/UsbDevice.h"

int main(int argc, char *argv[]) {
    set_qt_environment();
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;
    const QUrl url(mainQmlFile);
    QObject::connect(
                &engine, &QQmlApplicationEngine::objectCreated, &app,
                [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
            QCoreApplication::exit(-1);
        }
    }, Qt::QueuedConnection);

    Sound sound;
    UsbDevice usbDevice;

    QQmlContext *context = engine.rootContext();
    context->setContextProperty("sound", &sound);
    context->setContextProperty("usbDevice", &usbDevice);

    engine.addImportPath(QCoreApplication::applicationDirPath() + "/qml");
    engine.addImportPath(":/");
    engine.load(url);

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return QApplication::exec();
}
