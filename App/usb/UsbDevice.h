//
// Created by Capitang7 on 24/05/2026.
//

#ifndef CWKEYERAPP_USBDEVICE_H
#define CWKEYERAPP_USBDEVICE_H

#include <QDebug>
#include <QObject>
#include "libusb/libusb.h"


#include <iomanip>

class UsbDevice : public QObject {
  Q_OBJECT
  public:
    UsbDevice();
    ~UsbDevice();

    Q_INVOKABLE void list_devices();

    Q_INVOKABLE void connect_device();

  private:
    template<typename T> static std::string int_to_hex(T i);

};


#endif //CWKEYERAPP_USBDEVICE_H
