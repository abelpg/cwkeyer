//
// Created by Capitang7 on 25/05/2026.
//

#include "DeviceInterface.h"

DeviceInterface::DeviceInterface(int interface, int endpoint, int packetSize) {
  this->interface = interface;
  this->endpoint = endpoint;
  this->packetSize = packetSize;
}

DeviceInterface::DeviceInterface(DeviceInterface * deviceInterface) {
  this->interface = deviceInterface->interface;
  this->endpoint = deviceInterface->endpoint;
  this->packetSize = deviceInterface->packetSize;
};