//
// Created by Capitang7 on 25/05/2026.
//

#include "HidDevice.h"

HidDevice::HidDevice() {
  qDebug() << "HidDevice constructor called";
  const int rc = hid_init();
  assert(rc >= 0);

  configuration = new Configuration();
}

std::set<Device> HidDevice::list_devices() {
  std::set<Device> devicesLocal;

  hid_device_info *cur_dev = hid_enumerate(0x0, 0x0);

  while (cur_dev) {
    qDebug() << "Device Found";
    qDebug() << "  type: " << int_to_hex(cur_dev->vendor_id) << " " << int_to_hex(cur_dev->product_id);
    qDebug() << "  path: " << cur_dev->path;


    Device vp(cur_dev->vendor_id, cur_dev->product_id, new std::string(cur_dev->path));
    devicesLocal.insert(vp);


    cur_dev = cur_dev->next;
  }

  hid_free_enumeration(cur_dev);

  return devicesLocal;
}


void HidDevice::detect_device() {
  std::set<Device> devices = list_devices();

  for (int i=0; i< 5 && detected_device == nullptr; i++) {
    std::set<Device> devicesNow = list_devices();

    for (auto dn : devicesNow) {
      bool found = false;
      for (auto d : devices) {
        if (d == dn) {
          found = true;
          break;
        }
      }
      if (!found) {
        detected_device = &dn;
        break;
      }
    }

    if (detected_device == nullptr) {
      Utils::sleep_for(2000);
    }
  }

  if (detected_device != nullptr) {
    configuration->putObject("device", detected_device->toJson());
    qDebug() << " Detected device " << int_to_hex(detected_device->vendor_id) << " " << int_to_hex(detected_device->product_id);
  } else {
    qDebug() << " No device detected";
  }



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