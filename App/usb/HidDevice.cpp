//
// Created by Capitang7 on 25/05/2026.
//

#include "HidDevice.h"

HidDevice::HidDevice() {
  qDebug() << "HidDevice constructor called";
  const int rc = hid_init();
  assert(rc >= 0);
}

std::set<Device> HidDevice::list_devices() {
  std::set<Device> devicesLocal;

  hid_device_info *devs = hid_enumerate(0x0, 0x0);
  hid_device_info *cur_dev = devs;

  while (cur_dev) {
    qDebug() << "Device Found";
    qDebug() << "  type: " << int_to_hex(cur_dev->vendor_id) << " " << int_to_hex(cur_dev->product_id);
    qDebug() << "  path: " << cur_dev->path;
    qDebug() << "  serial_number: " << cur_dev->serial_number;
    qDebug() << "  manufacturer_string: " << cur_dev->manufacturer_string;
    qDebug() << "  product_string: " << cur_dev->product_string;
    qDebug() << "  release_number: " << cur_dev->release_number;
    qDebug() << "  interface_number: " << cur_dev->interface_number;

    Device vp(cur_dev->vendor_id, cur_dev->product_id);
    devicesLocal.insert(vp);

    cur_dev = cur_dev->next;
  }
  hid_free_enumeration(devs);

  return devicesLocal;
}

HidDevice::~HidDevice() {
  qDebug() << "HidDevice destructor called";
  const int rc = hid_exit();
  assert(rc >= 0);
}

/**
 * Transforms a number to a hexadecimal representation
 * @tparam T With number
 * @param i with value
 * @return String with hex value.
 */
template< typename T > std::string HidDevice::int_to_hex( T i ) {
  std::stringstream stream;
  stream << "0x"
         << std::setfill ('0') << std::setw(sizeof(T)*2)
         << std::hex << i;
  return stream.str();
}