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

    bool detect_device();

    std::string get_current_device();

  private:
    template<typename T> static std::string int_to_hex(T i);

    Device *detected_device = nullptr;

    Configuration *configuration = nullptr;

    std::set<Device> list_devices();

};


#endif //CWKEYERAPP_HIDDEVICE_H
