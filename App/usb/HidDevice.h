#ifndef CWKEYERAPP_HIDDEVICE_H
#define CWKEYERAPP_HIDDEVICE_H


#include <QObject>
#include <iomanip>
#include <thread>
#include <string>
#include <iostream>
#include "Device.h"
#include "hidapi/hidapi.h"
#include "../utils/Utils.h"
#include "../utils/Logger.h"
#include "../configuration/Configuration.h"


class HidDevice  {

  public:
    HidDevice();
    ~HidDevice();

    Device *detectDevice();
    Device *initDevice();
    Device *connectDevice();
    Device *disconnectDevice();

    bool connected() const { return m_connected; }

    template<typename T> static std::string intToHex(T i);

  private:

    static const std::string CONFIG_NAME;

    std::thread     m_threadTask;
    hid_device     *m_hidDevice       = nullptr;
    Device         *m_detectedDevice  = nullptr;
    Configuration  *m_configuration   = nullptr;
    bool            m_connected        = false;

    void taskRunnable();
    std::set<Device> listDevices();

};


#endif //CWKEYERAPP_HIDDEVICE_H
