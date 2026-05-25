//
// Created by Capitang7 on 24/05/2026.
//

#include "Sound.h"


Sound::Sound() {
    qDebug() << "Sound constructor called";
}

Sound::~Sound() {
    qDebug() << "Sound destructor called";
}

void Sound::list_devices() {
    const QList<QAudioDevice>  deviceInfos = QMediaDevices::audioOutputs();
    for (const QAudioDevice &deviceInfo : deviceInfos) {
        qDebug() << "Device name: " << deviceInfo.id();
    }
}

