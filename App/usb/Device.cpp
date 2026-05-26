
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

Device::Device(int vendor_id, int product_id, DeviceInterface * device_interface) {
  this->vendor_id = vendor_id;
  this->product_id = product_id;
  this->device_interface = device_interface;
}
