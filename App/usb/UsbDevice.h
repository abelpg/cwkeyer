#ifndef CWKEYERAPP_USBDEVICE_H
#define CWKEYERAPP_USBDEVICE_H


#include "libusb/libusb.h"
#include <iomanip>
#include <thread>
#include <atomic>
#include <stdio.h>
#include <stdlib.h>
#include <functional>
#include <thread>
#include "Device.h"
#include "../utils/Utils.h"
#include "../configuration/Configuration.h"
#include "../utils/IDitDah.h"
#include "../utils/Logger.h"

enum ClickValues {
  CLICK_LEFT  = 0x01,
  CLICK_RIGHT = 0x02,
  CLICK_BOTH  = 0x03
};

class UsbDevice {
  public:

    UsbDevice();
    UsbDevice(IDitDah *ditDah);
    ~UsbDevice();

    Device *detectDevice();
    Device *initDevice();
    Device *connectDevice();
    Device *disconnectDevice();

    bool connected() const { return m_connected; }

    void addDitDah(IDitDah *ditDah);

    template<typename T> static std::string intToHex(T i);

  private:

    static const std::string CONFIG_NAME;

    libusb_context       *m_context        = nullptr;
    std::thread           m_threadTask;
    Device               *m_detectedDevice = nullptr;
    bool                  m_dit            = false;
    bool                  m_dah            = false;
    bool                  m_connected      = false;
    std::list<IDitDah *>  m_ditDahList;

    void taskRunnable();

    static void cbInterrupt(libusb_transfer *transfer);

    std::set<Device>    manageDevices(Device *deviceToTry);
    DeviceInterface    *searchDeviceInterfaceAvailable(libusb_device *libusbDevice, Device *deviceToTry);
    bool                attachDevice(libusb_device_handle *deviceHandle, int interfaceNum);
    bool                tryToRead(Device *deviceToTry, DeviceInterface *iface);

    void sendDitDah(bool ditPressed, bool dahPressed);
    void sendDit(bool pressed);
    void sendDah(bool pressed);
};


#endif //CWKEYERAPP_USBDEVICE_H
