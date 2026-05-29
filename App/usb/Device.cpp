#include "Device.h"

bool operator==(const Device &left, const Device &right) {
  return left.vendorId == right.vendorId && left.productId == right.productId;
}

bool operator<(const Device &left, const Device &right) {
  return !(left.vendorId == right.vendorId && left.productId == right.productId);
}

Device::Device(int vendorId, int productId) {
  this->vendorId  = vendorId;
  this->productId = productId;
}

Device::Device(int vendorId, int productId, std::string *path) {
  this->vendorId  = vendorId;
  this->productId = productId;
  this->m_path    = path;
}

Device::Device(int vendorId, int productId, DeviceInterface *deviceInterface) {
  this->vendorId   = vendorId;
  this->productId  = productId;
  this->m_interface = new DeviceInterface(deviceInterface);
}

void Device::setPath(std::string *path) {
  this->m_path = path;
}

void Device::setInterface(DeviceInterface *deviceInterface) {
  if (deviceInterface != nullptr) {
    this->m_interface = new DeviceInterface(deviceInterface);
  }
}

DeviceInterface *Device::getInterface() {
  return m_interface;
}

std::string *Device::getPath() {
  return m_path;
}

Device::~Device() {
  //delete m_interface;
  //delete m_path;
}