//
// Created by Capitang7 on 24/05/2026.
//

#ifndef CWKEYERAPP_SOUND_H
#define CWKEYERAPP_SOUND_H


#include <QObject>
#include <QDebug>
#include <QtMultimedia/QMediaDevices>
#include <QtMultimedia/QAudioDevice>


class Sound : public QObject {
  Q_OBJECT
  public:
    Sound();
    ~Sound();

    Q_INVOKABLE void list_devices();
};


#endif //CWKEYERAPP_SOUND_H
