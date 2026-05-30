#include "UsbDevice.h"


const std::string UsbDevice::CONFIG_NAME = "device-usb";

UsbDevice::UsbDevice() {
  qDebug() << "UsbDevice constructor called";
  int rc = libusb_init(&m_context);
  assert(rc == LIBUSB_SUCCESS);
  libusb_set_option(m_context, LIBUSB_OPTION_USE_USBDK);
  libusb_set_debug(m_context, LIBUSB_LOG_LEVEL_INFO);
}

UsbDevice::UsbDevice(IDitDah *ditDah) : UsbDevice() {
  addDitDah(ditDah);
}

void UsbDevice::addDitDah(IDitDah *ditDah) {
  m_ditDahList.push_back(ditDah);
}

Device *UsbDevice::initDevice() {
  if (connectDevice() != nullptr) {
    return m_detectedDevice;
  }
  return nullptr;
}

Device *UsbDevice::connectDevice() {
  qDebug() << "UsbDevice connectDevice called";

  if (m_detectedDevice == nullptr) {
    QJsonObject *value = Configuration::getValue(CONFIG_NAME);
    if (value != nullptr) {
      m_detectedDevice = Device::fromJson(*value);
    }
  }

  if (m_detectedDevice != nullptr && !m_connected) {
    m_threadTask = std::thread(&UsbDevice::taskRunnable, this);
    Utils::sleepFor(500);
  }

  if (m_detectedDevice == nullptr || !m_connected) {
    Configuration::removeValue(CONFIG_NAME);
    qDebug() << "Failed to connect to device";
  }

  return m_detectedDevice;
}

Device *UsbDevice::disconnectDevice() {
  if (m_connected) {
    m_connected = false;
    m_threadTask.join();
  }
  return m_detectedDevice;
}

bool UsbDevice::attachDevice(libusb_device_handle *deviceHandle, int interfaceNum) {
  if (libusb_has_capability(LIBUSB_CAP_SUPPORTS_DETACH_KERNEL_DRIVER) &&
      libusb_kernel_driver_active(deviceHandle, interfaceNum) == 1) {
    const int rs = libusb_detach_kernel_driver(deviceHandle, interfaceNum);
    qDebug() << "Detached" << interfaceNum << "from kernel driver";
    return rs == LIBUSB_SUCCESS;
  }

  if (!libusb_has_capability(LIBUSB_CAP_SUPPORTS_DETACH_KERNEL_DRIVER)) {
    qDebug() << "Detaching kernel driver not supported on this platform";
  }
  return false;
}

bool UsbDevice::tryToRead(Device *deviceToTry, DeviceInterface *iface) {
  libusb_device_handle *deviceHandle =
      libusb_open_device_with_vid_pid(m_context, deviceToTry->vendorId, deviceToTry->productId);

  bool detached = attachDevice(deviceHandle, iface->interfaceNum);

  int rs = libusb_claim_interface(deviceHandle, iface->interfaceNum);

  bool readData = false;
  if (rs == LIBUSB_SUCCESS) {
    qDebug() << "Claimed interface";
    int read = 0, counter = 0;
    auto data = new unsigned char[iface->packetSize];

    while (libusb_interrupt_transfer(deviceHandle,
                                     (iface->endpoint | LIBUSB_ENDPOINT_IN),
                                     data, sizeof(data), &read, 1000) == 0
           && counter++ < 5 && !readData) {
      if (read > 0) {
        qDebug() << "Can read" << read;
        readData = true;
      }
    }
  }

  libusb_release_interface(deviceHandle, iface->interfaceNum);
  qDebug() << "released interface";
  if (detached) {
    rs = libusb_attach_kernel_driver(deviceHandle, iface->interfaceNum);
    assert(rs == LIBUSB_SUCCESS);
    qDebug() << "Reattached" << iface->interfaceNum << "to kernel driver";
  }
  libusb_close(deviceHandle);
  qDebug() << "Closed device";
  return readData;
}

std::set<Device> UsbDevice::manageDevices(Device *deviceToTry) {
  libusb_device **list  = nullptr;
  const ssize_t   count = libusb_get_device_list(m_context, &list);
  assert(count > 0);

  int              rc = 0;
  std::set<Device> devicesLocal;

  for (size_t idx = 0; idx < count; ++idx) {
    libusb_device            *usbDevice = list[idx];
    libusb_device_descriptor  desc      = {0};

    rc = libusb_get_device_descriptor(usbDevice, &desc);
    assert(rc == LIBUSB_SUCCESS);

    qDebug() << "DeviceClass: " << (int)desc.bDeviceClass;
    qDebug() << "IdVendor: "    << intToHex(desc.idVendor)
             << " IdProduct: "  << intToHex(desc.idProduct);

    if (deviceToTry != nullptr &&
        (desc.idVendor != deviceToTry->vendorId || desc.idProduct != deviceToTry->productId)) {
      continue;
    }

    Device vp(desc.idVendor, desc.idProduct);

    if (deviceToTry != nullptr) {
      vp.setInterface(searchDeviceInterfaceAvailable(usbDevice, deviceToTry));
      if (vp.getInterface() != nullptr) {
        devicesLocal.insert(vp);
      }
    } else {
      devicesLocal.insert(vp);
    }
  }

  libusb_free_device_list(list, 1);
  return devicesLocal;
}

DeviceInterface *UsbDevice::searchDeviceInterfaceAvailable(libusb_device *libusbDevice,
                                                            Device *deviceToTry) {
  libusb_config_descriptor *config;
  const int rc = libusb_get_active_config_descriptor(libusbDevice, &config);
  assert(rc == LIBUSB_SUCCESS);

  DeviceInterface *result = nullptr;

  for (int i = 0; i < config->bNumInterfaces && result == nullptr; i++) {
    auto usbIface = config->interface[i];

    for (int x = 0; x < usbIface.num_altsetting && result == nullptr; x++) {
      auto altSetting = usbIface.altsetting[x];

      for (int j = 0; j < altSetting.bNumEndpoints && result == nullptr; j++) {
        qDebug() << "Adding interface: " << altSetting.bInterfaceNumber
                 << " " << intToHex(altSetting.endpoint[j].bEndpointAddress)
                 << " " << altSetting.endpoint[j].wMaxPacketSize;

        DeviceInterface iface(altSetting.bInterfaceNumber,
                              altSetting.endpoint[j].bEndpointAddress,
                              altSetting.endpoint[j].wMaxPacketSize);

        if (tryToRead(deviceToTry, &iface)) {
          result = &iface;
        }
      }
    }
  }

  libusb_free_config_descriptor(config);
  return result;
}

Device *UsbDevice::detectDevice() {
  std::set<Device> devices = manageDevices(nullptr);

  delete m_detectedDevice;
  m_detectedDevice = nullptr;

  for (int i = 0; i < 10 && m_detectedDevice == nullptr; i++) {
    std::set<Device> devicesNow = manageDevices(nullptr);

    for (auto dn : devicesNow) {
      bool found = false;
      for (auto d : devices) {
        if (d == dn) {
          found = true;
          break;
        }
      }
      if (!found) {
        qDebug() << "Device detected: " << intToHex(dn.vendorId) << " " << intToHex(dn.productId);

        std::set<Device> finalDevice = manageDevices(&dn);
        for (auto d : finalDevice) {
          delete m_detectedDevice;
          m_detectedDevice = new Device(d.vendorId, d.productId,
                                        new DeviceInterface(d.getInterface()));
        }
        break;
      }
    }

    if (m_detectedDevice == nullptr) {
      Utils::sleepFor(1000);
    }
  }

  if (m_detectedDevice != nullptr) {
    Configuration::putObject(CONFIG_NAME, m_detectedDevice->toJson());
    qDebug() << " Detected device "
             << intToHex(m_detectedDevice->vendorId)
             << " " << intToHex(m_detectedDevice->productId);
  } else {
    qDebug() << " No device detected";
  }

  return m_detectedDevice;
}

/**
 * Transforms a number to a hexadecimal representation
 */
template<typename T> std::string UsbDevice::intToHex(T i) {
  std::stringstream stream;
  stream << "0x"
         << std::setfill('0') << std::setw(sizeof(T) * 2)
         << std::hex << i;
  return stream.str();
}

UsbDevice::~UsbDevice() {
  qDebug() << "UsbDevice destructor called";

  if (m_detectedDevice != nullptr) {
    delete m_detectedDevice;
  }
  libusb_exit(m_context);
}

void UsbDevice::taskRunnable() {
  qDebug() << "Starting task runnable";

  if (m_detectedDevice != nullptr) {
    int l_interface = m_detectedDevice->getInterface()->interfaceNum;
    int l_packeSize = m_detectedDevice->getInterface()->packetSize;
    int l_endpoint = m_detectedDevice->getInterface()->endpoint;


    libusb_device_handle *deviceHandle =
        libusb_open_device_with_vid_pid(m_context,
                                        m_detectedDevice->vendorId,
                                        m_detectedDevice->productId);

    bool detached = attachDevice(deviceHandle, l_interface);

    int rs = libusb_claim_interface(deviceHandle, l_interface);
    if (rs == LIBUSB_SUCCESS) {
      libusb_transfer *transfer = libusb_alloc_transfer(0);
      unsigned char    buffer[l_packeSize];

      libusb_fill_interrupt_transfer(
          transfer,
          deviceHandle,
          l_endpoint,
          buffer,
          l_packeSize,
          cbInterrupt,
          this,
          1000
      );

      libusb_submit_transfer(transfer);
      m_connected = true;

      while (m_connected) {
        libusb_handle_events(m_context);
      }

      if (detached) {
        rs = libusb_attach_kernel_driver(deviceHandle, l_interface);
        assert(rs == LIBUSB_SUCCESS);
        qDebug() << "Reattached" << l_interface << "to kernel driver";
      }
      libusb_release_interface(deviceHandle, l_interface);
      libusb_close(deviceHandle);
    }
  }
}

void UsbDevice::cbInterrupt(libusb_transfer *transfer) {
  if (transfer->status == LIBUSB_TRANSFER_COMPLETED) {
    UsbDevice *usbDevice = static_cast<UsbDevice *>(transfer->user_data);

    if (transfer->buffer[0] == CLICK_BOTH ||
        (transfer->buffer[2] > 0 && transfer->buffer[3] > 0)) {
      usbDevice->sendDitDah(true, true);
    } else if (transfer->buffer[0] == CLICK_LEFT || transfer->buffer[2] > 0) {
      usbDevice->sendDitDah(true, false);
    } else if (transfer->buffer[0] == CLICK_RIGHT || transfer->buffer[3] > 0) {
      usbDevice->sendDitDah(false, true);
    } else {
      usbDevice->sendDitDah(false, false);
    }

    // Re-submit the transfer to keep listening (looping)
    libusb_submit_transfer(transfer);

  } else if (transfer->status == LIBUSB_TRANSFER_TIMED_OUT) {
    qDebug() << "Transfer timed out, resubmitting...";
    libusb_submit_transfer(transfer);
  } else {
    fprintf(stderr, "Transfer finished with status: %d\n", transfer->status);
    libusb_free_transfer(transfer);
  }
}

void UsbDevice::sendDah(bool pressed) {
  for (IDitDah *ditDah : m_ditDahList) {
    ditDah->onDah(pressed);
  }
}

void UsbDevice::sendDit(bool pressed) {
  for (IDitDah *ditDah : m_ditDahList) {
    ditDah->onDit(pressed);
  }
}

void UsbDevice::sendDitDah(bool ditPressed, bool dahPressed) {
  if (ditPressed && !m_dit) {
    m_dit = true;
    sendDit(true);
  } else if (!ditPressed && m_dit) {
    m_dit = false;
    sendDit(false);
  }

  if (dahPressed && !m_dah) {
    m_dah = true;
    sendDah(true);
  } else if (!dahPressed && m_dah) {
    m_dah = false;
    sendDah(false);
  }

}
