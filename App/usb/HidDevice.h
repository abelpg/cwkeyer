#ifndef CWKEYERAPP_HIDDEVICE_H
#define CWKEYERAPP_HIDDEVICE_H

#include <QDebug>
#include <QObject>

#include <string>
#include <iostream>
#include "Device.h"
#include "hidapi/hidapi.h"
#include "../utils/Utils.h"
#include "../configuration/Configuration.h"
#include <iomanip>

class HidDevice  {

  public:
    HidDevice();
    ~HidDevice();

    Device * detect_device();

    Device * init_device();

    bool connect_device();

    bool disconnect_device();

    template<typename T> static std::string int_to_hex(T i);

  private:

    static const std::string CONFIG_NAME;

    hid_device * hid_device;

    Device *detected_device = nullptr;

    Configuration *configuration = nullptr;

    std::set<Device> list_devices();

};


#endif //CWKEYERAPP_HIDDEVICE_H
