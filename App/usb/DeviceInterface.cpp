//
// Created by Capitang7 on 25/05/2026.
//

#include "DeviceInterface.h"

DeviceInterface::DeviceInterface(int interfaceNum, int endpoint, int packetSize) {
  this->interfaceNum = interfaceNum;
  this->endpoint     = endpoint;
  this->packetSize   = packetSize;
}

DeviceInterface::DeviceInterface(DeviceInterface *deviceInterface) {
  this->interfaceNum = deviceInterface->interfaceNum;
  this->endpoint     = deviceInterface->endpoint;
  this->packetSize   = deviceInterface->packetSize;
};