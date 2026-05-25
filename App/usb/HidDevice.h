#ifndef CWKEYERAPP_HIDDEVICE_H
#define CWKEYERAPP_HIDDEVICE_H

#include <QDebug>
#include <QObject>

#include "VendorProduct.h"
#include "hidapi/hidapi.h"

class HidDevice  : public QObject {
  Q_OBJECT
  public:

  HidDevice();
  ~HidDevice();

  Q_INVOKABLE std::set<VendorProduct> list_devices();
};


#endif //CWKEYERAPP_HIDDEVICE_H
