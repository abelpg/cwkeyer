#ifndef CWKEYERAPP_USBDEVICE_H
#define CWKEYERAPP_USBDEVICE_H

#include <QDebug>
#include <iomanip>

#include "libusb/libusb.h"
#include "../utils/Utils.h"
#include "Device.h"

class UsbDevice  {
  public:

    UsbDevice();
    ~UsbDevice();

    std::set<Device> list_devices();

    void connect_device();

    void detect_device();

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
    Device *detected_device = nullptr;

    template<typename T> static std::string int_to_hex(T i);


};


#endif //CWKEYERAPP_USBDEVICE_H
