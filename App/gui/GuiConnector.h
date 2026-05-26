
#ifndef CWKEYERAPP_GUICONNECTOR_H
#define CWKEYERAPP_GUICONNECTOR_H

#include <QObject>
#include <QVariant>
#include <QObject>
#include <iostream>
#include <QVariant>
#include "../sound/Sound.h"
#include "../usb/HidDevice.h"



class GuiConnector : public QObject{
  Q_OBJECT
  public:
    explicit  GuiConnector(QObject *parent = 0);

    Q_INVOKABLE void detect_device();

  signals:
      void device_updated(QVariant varData);

  private:
    Sound* sound;
    HidDevice* device;
};


#endif //CWKEYERAPP_GUICONNECTOR_H
