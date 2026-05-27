#ifndef CWKEYERAPP_USBDEVICE_H
#define CWKEYERAPP_USBDEVICE_H

#include <QDebug>


#include "libusb/libusb.h"
#include <iomanip>
#include <thread>
#include <atomic>
#include <stdio.h>
#include <stdlib.h>
#include <functional>

#include "Device.h"
#include "../utils/Utils.h"
#include "../configuration/Configuration.h"
#include "../utils/IDitDah.h"

typedef std::function<void (boolean)> boolean_callback;

enum ClickValues {
  CLICK_LEFT = 0x01,
  CLICK_RIGHT = 0x02,
  CLICK_BOTH = 0x03
};

class UsbDevice  {
  public:

    UsbDevice();
    UsbDevice(IDitDah* dit_dah);
    ~UsbDevice();

    Device * detect_device();

    Device * init_device();

    Device * connect_device();

    Device * disconnect_device();

    template<typename T> static std::string int_to_hex(T i);

  private:

    static const std::string CONFIG_NAME;

    /**
     * Usb context.
     */
    libusb_context *context = nullptr;

    std::thread thread_task;

    Device *detected_device = nullptr;

    Configuration *configuration = nullptr;

    bool dit, dah = false;

    IDitDah * dit_dah;

    ///////////////
    void task_runnable();

    static void cb_interrupt( libusb_transfer *transfer) ;

    std::set<Device> manage_devices(Device *deviceToTry);

    DeviceInterface * search_device_interface_available(libusb_device *libusb_device, Device * deviceToTry);

    bool attach_device(libusb_device_handle *deviceTemp, int interface);

    bool try_to_read(Device * deviceToTry, DeviceInterface *interface);

    void sed_dih_dah(bool dit, bool dah);


};


#endif //CWKEYERAPP_USBDEVICE_H
