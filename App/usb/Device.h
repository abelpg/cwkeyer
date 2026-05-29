#ifndef CWKEYERAPP_DEVICE_H
#define CWKEYERAPP_DEVICE_H

/*
*TESTED_DEVICES = [
{"vendor_id": 0x413d, "product_id": 0x2107, "interface": 0, "endpoint": 0x81, "max_packet_size": 8}, # Vail
{"vendor_id": 0x413d, "product_id": 0x2107, "interface": 1, "endpoint": 0x82, "max_packet_size": 4}  # Left click/right
]
*/

#include <QDebug>
#include <string>
#include <iostream>
#include <stdio.h>
#include <QJsonObject>
#include <list>
#include "DeviceInterface.h"


class Device {
  public:

    int vendorId  = 0;
    int productId = 0;

    Device(int vendorId, int productId);
    Device(int vendorId, int productId, std::string *path);
    Device(int vendorId, int productId, DeviceInterface *deviceInterface);
    ~Device();

    friend bool operator==(const Device &left, const Device &right);
    friend bool operator< (const Device &left, const Device &right);

    void setPath(std::string *path);
    void setInterface(DeviceInterface *deviceInterface);
    std::string    *getPath();
    DeviceInterface *getInterface();

    QJsonObject toJson() const {
      QJsonObject jsonObject;
      jsonObject["vendor_id"]  = vendorId;
      jsonObject["product_id"] = productId;

      if (m_path != nullptr) {
        jsonObject["path"] = QString::fromStdString(*m_path);
      }

      if (m_interface != nullptr) {
        jsonObject["interface"] = m_interface->toJson();
      }

      return jsonObject;
    };

    static Device *fromJson(QJsonObject jsonObject) {
      int vendorId  = jsonObject["vendor_id"].toInt();
      int productId = jsonObject["product_id"].toInt();

      if (jsonObject.contains("path")) {
        return new Device(vendorId, productId,
                          new std::string(jsonObject["path"].toString().toStdString()));
      }

      if (jsonObject.contains("interface")) {
        return new Device(vendorId, productId,
                          DeviceInterface::fromJson(jsonObject["interface"].toObject()));
      }

      return new Device(vendorId, productId);
    }

  private:
    std::string     *m_path      = nullptr;
    DeviceInterface *m_interface = nullptr;

};


#endif //CWKEYERAPP_DEVICE_H
