//
// Created by Capitang7 on 24/05/2026.
//

#ifndef CWKEYERAPP_USBDEVICE_H
#define CWKEYERAPP_USBDEVICE_H

#include <QDebug>
#include <QObject>
#include "libusb/libusb.h"

#include <functional>
#include <iomanip>

#include "VendorProduct.h"

class UsbDevice : public QObject {
  Q_OBJECT
  public:

    UsbDevice();
    ~UsbDevice();

    Q_INVOKABLE std::list<VendorProduct> list_devices();

    Q_INVOKABLE void connect_device();

  private:
  /*
*TESTED_DEVICES = [
{"vendor_id": 0x413d, "product_id": 0x2107, "interface": 0, "endpoint": 0x81, "max_packet_size": 8}, # Vail
{"vendor_id": 0x413d, "product_id": 0x2107, "interface": 1, "endpoint": 0x82, "max_packet_size": 4}  # Left click/right
]
 */

    const int VID = 0x413d;
    const int PID = 0x2107;
    const int INTERFACE = 1;
    const int ENDPOINT = 0x82;

    std::list<VendorProduct> devices;

    /**
     * Usb context.
     */
    libusb_context *context = nullptr;

    /**
     * Callback handle to perform hotplug.
     */
    libusb_hotplug_callback_handle callback_handle;

    template<typename T> static std::string int_to_hex(T i);

    static void print_configuration(libusb_config_descriptor *config);


};


#endif //CWKEYERAPP_USBDEVICE_H
