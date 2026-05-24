//
// Created by Capitang7 on 24/05/2026.
//

#ifndef CWKEYERAPP_USBDEVICE_H
#define CWKEYERAPP_USBDEVICE_H

#include <QDebug>
#include <QObject>
#include "libusb/libusb.h"
#include <stdio.h>
#include <string.h>

class UsbDevice : public QObject {
  Q_OBJECT
  public:
    UsbDevice();
    ~UsbDevice();

    Q_INVOKABLE void list_devices();

  private:
    static void print_devs(libusb_device **devs, int verbose);
};


#endif //CWKEYERAPP_USBDEVICE_H
