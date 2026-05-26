//
// Created by Capitang7 on 25/05/2026.
//

#include "HidDevice.h"
const std::string HidDevice::CONFIG_NAME = "device";
HidDevice::HidDevice() {
  qDebug() << "HidDevice constructor called";
  const int rc = hid_init();
  assert(rc >= 0);

  configuration = new Configuration();
}

Device * HidDevice::init_device() {
  return detected_device;
}

bool HidDevice::connect_device() {

  if (detected_device == nullptr) {
    ConfigurationValue*  value =  configuration->getValue(CONFIG_NAME) ;
    if (value != nullptr) {
      detected_device = Device::fromJson(*value->value);
    }
  }

  if (detected_device != nullptr) {

    if (!detected_device->connected) {
      hid_device = hid_open_path(detected_device->path->c_str());
      if (hid_device != nullptr) {
        detected_device->connected = true;
        qDebug() << "Connected to device " << int_to_hex(detected_device->vendor_id) << " " << int_to_hex(detected_device->product_id);
      }
    }

    return detected_device->connected;
  }

  Configuration::removeValue(CONFIG_NAME);
  qDebug() << "Failed to connect to device";

  return false;
}

bool HidDevice::disconnect_device() {

  if (detected_device->connected) {
    hid_close(hid_device);
    detected_device->connected = false;
    return true;
  }

  return false;
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


 Device * HidDevice::detect_device() {
  std::set<Device> devices = list_devices();

  for (int i=0; i< 10 && detected_device == nullptr; i++) {
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
      Utils::sleep_for(1000);
    }
  }

  if (detected_device != nullptr) {
    configuration->putObject(CONFIG_NAME, detected_device->toJson());
    qDebug() << " Detected device " << int_to_hex(detected_device->vendor_id) << " " << int_to_hex(detected_device->product_id);

  } else {
    qDebug() << " No device detected";
  }

  return detected_device;
}

HidDevice::~HidDevice() {
  qDebug() << "HidDevice destructor called";
  if (detected_device != nullptr) {
    delete detected_device;
  }
  if (hid_device != nullptr) {
    hid_close(hid_device);
  }
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