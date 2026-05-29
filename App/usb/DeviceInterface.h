#ifndef CWKEYERAPP_DEVICEINTERFACE_H
#define CWKEYERAPP_DEVICEINTERFACE_H

#include <iostream>
#include <stdio.h>
#include <QJsonObject>

class DeviceInterface {

  public:
    int interfaceNum;
    int endpoint;
    int packetSize;

    DeviceInterface(DeviceInterface *deviceInterface);
    DeviceInterface(int interfaceNum, int endpoint, int packetSize);

    QJsonObject toJson() const {
      QJsonObject jsonObject;
      jsonObject["interface"]  = interfaceNum;
      jsonObject["endpoint"]   = endpoint;
      jsonObject["packetSize"] = packetSize;
      return jsonObject;
    };

    static DeviceInterface *fromJson(QJsonObject jsonObject) {
      return new DeviceInterface(
          jsonObject["interface"].toInt(),
          jsonObject["endpoint"].toInt(),
          jsonObject["packetSize"].toInt());
    }
};


#endif //CWKEYERAPP_DEVICEINTERFACE_H
