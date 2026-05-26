#ifndef CWKEYERAPP_USBDEVICE_H
#define CWKEYERAPP_USBDEVICE_H

#include <QDebug>


#include "libusb/libusb.h"
#include "Device.h"
#include "../utils/Utils.h"
#include "../configuration/Configuration.h"
#include <iomanip>
#include <thread>
class UsbDevice  {
  public:

    UsbDevice();
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

    libusb_device_handle *device = nullptr;

    Device *detected_device = nullptr;

    Configuration *configuration = nullptr;

    ///////////////
    void task_runnable();

    std::set<Device> manage_devices(Device *deviceToTry);

    DeviceInterface * search_device_interface_available(libusb_device *libusb_device, Device * deviceToTry);

    bool attach_device(libusb_device_handle *deviceTemp, int interface);

    bool try_to_read(Device * deviceToTry, DeviceInterface *interface);


};


#endif //CWKEYERAPP_USBDEVICE_H
