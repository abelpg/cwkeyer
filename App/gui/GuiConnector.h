
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

    Q_INVOKABLE void init_device();

    Q_INVOKABLE void detect_device();

    Q_INVOKABLE void connect_device();

    Q_INVOKABLE void disconnect_device();

  //Q_PROPERTY(QString text MEMBER m_text NOTIFY device_updated)
  signals:
      void device_initiated(QVariant varData);
      void device_updated(QVariant varData);
      void device_connected();
      void device_disconnected();


  private:
    Sound* sound;
    HidDevice* device;
};


#endif //CWKEYERAPP_GUICONNECTOR_H
