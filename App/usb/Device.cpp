
#include "Device.h"

bool operator== (const Device &left, const Device &right) {
  return left.vendor_id == right.vendor_id && left.product_id == right.product_id;
}

bool operator< (const Device &left, const Device &right) {
  return !(left.vendor_id == right.vendor_id && left.product_id == right.product_id) ;
}


Device::Device(int vendor_id, int product_id)   {
  this->vendor_id = vendor_id;
  this->product_id = product_id;
}

Device::Device(int vendor_id, int product_id, std::string *path)   {
  this->vendor_id = vendor_id;
  this->product_id = product_id;
  this->path = path;
}

Device::Device(int vendor_id, int product_id, int interface, int endpoint, int packet_size) {
  this->vendor_id = vendor_id;
  this->product_id = product_id;
  this->device_interface = new DeviceInterface(interface, endpoint, packet_size);
}

void Device::addInterface(int interface, int endpoint, int packetSize) {
  if (available_interfaces == nullptr) {
    available_interfaces = new std::list<DeviceInterface>();
  }
  DeviceInterface deviceInterface(interface, endpoint, packetSize);
  available_interfaces->push_back(deviceInterface);
}

Device::~Device() {

  if (available_interfaces != nullptr) {
    for (auto it = available_interfaces->begin(); it != available_interfaces->end(); ++it) {
      it  = available_interfaces->erase(it);
    }
    delete available_interfaces;
  }

  delete device_interface;

}