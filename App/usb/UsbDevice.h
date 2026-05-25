//
// Created by Capitang7 on 24/05/2026.
//

#ifndef CWKEYERAPP_USBDEVICE_H
#define CWKEYERAPP_USBDEVICE_H

#include <QDebug>
#include <QObject>
#include "libusb/libusb.h"
#include <iomanip>

#include <unistd.h>

#include "Device.h"

class UsbDevice : public QObject {
  Q_OBJECT
  public:

    UsbDevice();
    ~UsbDevice();

    Q_INVOKABLE std::set<Device> list_devices();

    Q_INVOKABLE void connect_device();

    Q_INVOKABLE Device* detect_device();

  private:

    const int VID = 0x413d;
    const int PID = 0x2107;
    const int INTERFACE = 1;
    const int ENDPOINT = 0x82;

    /**
     * Usb context.
     */
    libusb_context *context = nullptr;

    /**
     *
     */
    Device *detectedDevice = nullptr;

    template<typename T> static std::string int_to_hex(T i);

    static void sleep_for(int milliseconds);

};


#endif //CWKEYERAPP_USBDEVICE_H
