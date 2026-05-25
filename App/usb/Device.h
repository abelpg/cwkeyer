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
#include <DeviceInterface.h>


class Device {
  public:
    std::string * path = nullptr;
    int vendor_id = 0;
    int product_id = 0;


    DeviceInterface * device_interface = nullptr;

    std::list<DeviceInterface> *available_interfaces{};

    Device(int vendor_id, int product_id);
    Device(int vendor_id, int product_id, std::string *path);
    Device(int vendor_id, int product_id, int interface, int endpoint, int packet_size);
    ~Device();

    void addInterface(int interface, int endpoint, int packetSize);


    friend bool operator== (const Device &left, const Device &right);

    friend bool operator< (const Device &left, const Device &right);

    QJsonObject toJson() const {
      QJsonObject jsonObject;
      jsonObject["vendor_id"] = vendor_id;
      jsonObject["product_id"] = product_id;

      if (path != nullptr) {
        jsonObject["path"] = QString::fromStdString(*path);
      }

      return jsonObject;
    };

    Device fromJson(QJsonObject jsonObject) const {
      int vendor_id = jsonObject["vendor_id"].toInt();
      int product_id = jsonObject["product_id"].toInt();

      if (jsonObject.contains("path")) {
        return Device(vendor_id, product_id, new std::string(jsonObject["path"].toString().toStdString()));
      }

      return Device(vendor_id, product_id);
    }


};


#endif //CWKEYERAPP_DEVICE_H
