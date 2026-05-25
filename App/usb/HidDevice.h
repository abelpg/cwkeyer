#ifndef CWKEYERAPP_HIDDEVICE_H
#define CWKEYERAPP_HIDDEVICE_H

#include <QDebug>
#include <QObject>

#include "Device.h"
#include "hidapi/hidapi.h"
#include <iomanip>

class HidDevice  : public QObject {
  Q_OBJECT
  public:
    HidDevice();
    ~HidDevice();

    Q_INVOKABLE std::set<Device> list_devices();

  private:
    template<typename T> static std::string int_to_hex(T i);
};


#endif //CWKEYERAPP_HIDDEVICE_H
