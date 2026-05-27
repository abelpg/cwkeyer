
#ifndef CWKEYERAPP_GUICONNECTOR_H
#define CWKEYERAPP_GUICONNECTOR_H

#include <QObject>
#include <QVariant>
#include <QObject>
#include <iostream>
#include <QVariant>
#include "../sound/Sound.h"
#include "../usb/UsbDevice.h"
#include "../keyer/Keyer.h"



class GuiConnector : public QObject{
  Q_OBJECT

  public:
    explicit  GuiConnector(QObject *parent = 0);

    Q_INVOKABLE void init_device();

    Q_INVOKABLE void detect_device();

    Q_INVOKABLE void connect_device();

    Q_INVOKABLE void disconnect_device();

    Q_INVOKABLE void quit();

  //Q_PROPERTY(QString text MEMBER m_text NOTIFY device_updated)
  signals:
      void device_updated(QVariant varData);

  private:
    Sound* sound;
    UsbDevice* device;
    Keyer* keyer;

    void send_device_updated(Device * device);


};


#endif //CWKEYERAPP_GUICONNECTOR_H
