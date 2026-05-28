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
#include <thread>

class HidDevice  {

  public:
    HidDevice();
    ~HidDevice();

    Device * detect_device();

    Device * init_device();

    Device * connect_device();

    Device * disconnect_device();

    bool  connected()             const { return _connected; }

    template<typename T> static std::string int_to_hex(T i);

  private:

    static const std::string CONFIG_NAME;

    std::thread thread_task;

    hid_device * hid_device;

    Device *detected_device = nullptr;

    Configuration *configuration = nullptr;


    bool _connected = false;


    ///////////////
    void task_runnable();

    std::set<Device> list_devices();

};


#endif //CWKEYERAPP_HIDDEVICE_H
