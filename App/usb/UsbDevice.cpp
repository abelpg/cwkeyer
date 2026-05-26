

#include "UsbDevice.h"


const std::string UsbDevice::CONFIG_NAME = "device-usb";

UsbDevice::UsbDevice() {
  qDebug() << "UsbDevice constructor called";
  int rc = libusb_init(&context);
  assert(rc == LIBUSB_SUCCESS);
  libusb_set_option(context, LIBUSB_OPTION_USE_USBDK);
  libusb_set_debug(context, LIBUSB_LOG_LEVEL_INFO);
}

Device * UsbDevice::init_device() {
  if (connect_device() != nullptr) {
    return detected_device;
  }
  return nullptr;
}

Device* UsbDevice::connect_device() {
  qDebug() << "UsbDevice connect_device called";

  if (detected_device == nullptr) {
    ConfigurationValue*  value =  configuration->getValue(CONFIG_NAME) ;
    if (value != nullptr) {
      detected_device = Device::fromJson(*value->value);
    }
  }

  if (detected_device != nullptr) {
    device = libusb_open_device_with_vid_pid(context, detected_device->vendor_id, detected_device->product_id);

  }

  if (detected_device == nullptr || !detected_device->connected) {
    Configuration::removeValue(CONFIG_NAME);
    qDebug() << "Failed to connect to device";
  }

  return detected_device;
}


Device * UsbDevice::disconnect_device() {

  if (detected_device->connected) {
    detected_device->connected = false;
    libusb_release_interface(device, detected_device->device_interface->interface); //release the claimed interface
    libusb_close(device); //close the device we opened
  }

  return detected_device;
}

bool UsbDevice::attach_device(libusb_device_handle *deviceTemp, int interface) {
  if (libusb_has_capability (LIBUSB_CAP_SUPPORTS_DETACH_KERNEL_DRIVER) && libusb_kernel_driver_active(deviceTemp, interface) == 1) { //find out if kernel driver is attached
    const int rs = libusb_detach_kernel_driver(deviceTemp, interface); //detach it
    qDebug() << "Detached" << interface << "from kernel driver";
    return rs == LIBUSB_SUCCESS;
  }

  if (!libusb_has_capability (LIBUSB_CAP_SUPPORTS_DETACH_KERNEL_DRIVER)) {
    qDebug() << "Detaching kernel driver not supported on this platform";
  }

  return false;
}


bool UsbDevice::try_to_read(Device * deviceToTry , DeviceInterface *interface) {
  libusb_device_handle *deviceTemp =  libusb_open_device_with_vid_pid(context, deviceToTry->vendor_id, deviceToTry->product_id);


  bool detached = attach_device(deviceTemp, interface->interface);

  int rs=0;
  rs = libusb_claim_interface(deviceTemp,  interface->interface); //claim interface 0 (the first) of device (desired device FX3 has only 1)

  bool readData = false;
  if (rs == LIBUSB_SUCCESS) {
    qDebug() << "Claimed interface";
    int read = 0, counter=0;
    auto data = new unsigned char[interface->packetSize];

    while (libusb_interrupt_transfer(deviceTemp, (interface->endpoint | LIBUSB_ENDPOINT_IN), data, sizeof(data), &read, 1000) == 0 && counter++ < 5 && !readData) {
      if (read > 0) {
        qDebug() << "Can read" <<read;
        readData = true;
      }
    }

  }

  libusb_release_interface(deviceTemp, interface->interface); //release the claimed interface
  qDebug() << "released interface";
  if (detached) {
    rs = libusb_attach_kernel_driver(deviceTemp, interface->interface); //reattach it
    assert(rs == LIBUSB_SUCCESS);
    qDebug() << "Reattached" << interface->interface << "to kernel driver";
  }
  libusb_close(deviceTemp); //close the device we opened
  qDebug() << "Closed device";
  return readData;
}



std::set<Device> UsbDevice::manage_devices(Device *deviceToTry) {

  libusb_device **list = nullptr;
  const ssize_t count = libusb_get_device_list(context, &list);
  assert(count > 0);

  int rc = 0;
  std::set<Device> devicesLocal;
  for (size_t idx = 0; idx < count; ++idx) {
    libusb_device *device = list[idx];
    libusb_device_descriptor desc = {0};

    rc = libusb_get_device_descriptor(device, &desc);
    assert(rc == LIBUSB_SUCCESS);

    qDebug() << "DeviceClass: " << (int) desc.bDeviceClass ;
    qDebug()  << "IdVendor: "  << int_to_hex(desc.idVendor) << " IdProduct: " << int_to_hex(desc.idProduct);

    if (deviceToTry != nullptr && (desc.idVendor != deviceToTry->vendor_id || desc.idProduct != deviceToTry->product_id)) {
      continue;
    }

    Device vp(desc.idVendor, desc.idProduct);

    if (deviceToTry != nullptr) {
      vp.device_interface = search_device_interface_available(device, deviceToTry);
      if (vp.device_interface != nullptr) {
        devicesLocal.insert(vp);
      }
    } else {
      devicesLocal.insert(vp);
    }

  }

  libusb_free_device_list(list, 1);

  return devicesLocal;
}

DeviceInterface* UsbDevice::search_device_interface_available(libusb_device *libusb_device, Device * deviceToTry) {

  libusb_config_descriptor *config;

  const int rc = libusb_get_active_config_descriptor(libusb_device, &config);
  assert(rc == LIBUSB_SUCCESS);

  DeviceInterface * result = nullptr;
  for (int i=0; i< config->bNumInterfaces && result == nullptr; i++) {

    auto interface = config->interface[i];

    for (int x=0; x < interface.num_altsetting && result == nullptr; x++) {
      auto alt_setting = interface.altsetting[x];

      for (int j=0; j< alt_setting.bNumEndpoints && result == nullptr; j++) {
        qDebug() << "Adding interfaz: " <<  alt_setting.bInterfaceNumber  << " " << alt_setting.endpoint[j].bEndpointAddress << " " << alt_setting.endpoint[j].wMaxPacketSize;
        DeviceInterface iface(alt_setting.bInterfaceNumber,alt_setting.endpoint[j].bEndpointAddress, alt_setting.endpoint[j].wMaxPacketSize);

        if (try_to_read(deviceToTry, &iface)) {
          result = &iface;
        }

      }
    }
  }

  libusb_free_config_descriptor(config);

  return result;
}


Device *  UsbDevice::detect_device() {
  std::set<Device> devices = manage_devices(nullptr);

  delete detected_device;
  for (int i=0; i< 10 && detected_device == nullptr; i++) {
    std::set<Device> devicesNow = manage_devices(nullptr);

    for (auto dn : devicesNow) {
      bool found = false;
      for (auto d : devices) {
        if (d == dn) {
          found = true;
          break;
        }
      }
      if (!found) {

        std::set<Device> finalDevice = manage_devices(&dn);
        for (auto d : finalDevice) {
          detected_device = &d;
        }
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


/**
 * Transforms a number to a hexadecimal representation
 * @tparam T With number
 * @param i with value
 * @return String with hex value.
 */
template< typename T > std::string UsbDevice::int_to_hex( T i ) {
  std::stringstream stream;
  stream << "0x"
         << std::setfill ('0') << std::setw(sizeof(T)*2)
         << std::hex << i;
  return stream.str();
}


UsbDevice::~UsbDevice() {
  qDebug() << "UsbDevice destructor called";

  if (detected_device != nullptr) {
    delete detected_device;
  }
  libusb_exit(context);

}

void UsbDevice::task_runnable() {
  qDebug() << "Starting task runnable";
  unsigned char buff[8];;

  // auto data = new unsigned char[4];
  // while (libusb_interrupt_transfer(device, (ENDPOINT | LIBUSB_ENDPOINT_IN), data, sizeof(data), &read, 1000) == 0 && counter++ < 5) {
  //   qDebug() << "Data read: " << data[0] << " " << data[1] << " " << data[2] << " " << data[3];
  // }
  // delete[] data; //delete the allocated memory for data
  //

  while (detected_device != nullptr && detected_device->connected) {
    //int res = hid_read_timeout(hid_device, buff, 8,5000);
    //qDebug() << res;
  }

}