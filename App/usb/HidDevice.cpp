//
// Created by Capitang7 on 25/05/2026.
//

#include "HidDevice.h"

const std::string HidDevice::CONFIG_NAME = "device";

HidDevice::HidDevice() {
  qDebug() << "HidDevice constructor called";
  const int rc = hid_init();
  assert(rc >= 0);
  m_configuration = new Configuration();
}

Device *HidDevice::initDevice() {
  if (connectDevice()) {
    return m_detectedDevice;
  }
  return nullptr;
}

Device *HidDevice::connectDevice() {
  if (m_detectedDevice == nullptr) {
    QJsonObject *value = m_configuration->getValue(CONFIG_NAME);
    if (value != nullptr) {
      m_detectedDevice = Device::fromJson(*value);
    }
  }

  if (m_detectedDevice != nullptr) {
    if (!m_connected) {
      m_hidDevice = hid_open_path(m_detectedDevice->getPath()->c_str());
      if (m_hidDevice != nullptr) {
        hid_set_nonblocking(m_hidDevice, 0);
        m_connected = true;

        qDebug() << "Connected to device "
                 << intToHex(m_detectedDevice->vendorId)
                 << " " << intToHex(m_detectedDevice->productId);

        if (m_threadTask.joinable()) {
          qDebug() << "Thread already running";
          m_threadTask.detach();
        }
        m_threadTask = std::thread(&HidDevice::taskRunnable, this);
      }
    } else {
      qDebug() << "Already connected";
    }
  }

  if (m_detectedDevice == nullptr || !m_connected) {
    Configuration::removeValue(CONFIG_NAME);
    qDebug() << "Failed to connect to device";
  }

  return m_detectedDevice;
}

Device *HidDevice::disconnectDevice() {
  if (m_connected) {
    m_connected = false;
    hid_close(m_hidDevice);
  }
  return m_detectedDevice;
}

std::set<Device> HidDevice::listDevices() {
  std::set<Device> devicesLocal;

  hid_device_info *curDev = hid_enumerate(0x0, 0x0);

  while (curDev) {
    qDebug() << "Device Found";
    qDebug() << "  type: " << intToHex(curDev->vendor_id) << " " << intToHex(curDev->product_id);
    qDebug() << "  path: " << curDev->path;
    qDebug() << "  interface: " << curDev->interface_number;

    Device vp(curDev->vendor_id, curDev->product_id, new std::string(curDev->path));
    devicesLocal.insert(vp);

    curDev = curDev->next;
  }

  hid_free_enumeration(curDev);
  return devicesLocal;
}

Device *HidDevice::detectDevice() {
  std::set<Device> devices = listDevices();

  delete m_detectedDevice;
  m_detectedDevice = nullptr;

  for (int i = 0; i < 10 && m_detectedDevice == nullptr; i++) {
    std::set<Device> devicesNow = listDevices();

    for (auto dn : devicesNow) {
      bool found = false;
      for (auto d : devices) {
        if (d == dn) {
          found = true;
          break;
        }
      }
      if (!found) {
        m_detectedDevice = &dn;
        break;
      }
    }

    if (m_detectedDevice == nullptr) {
      Utils::sleepFor(1000);
    }
  }

  if (m_detectedDevice != nullptr) {
    m_configuration->putObject(CONFIG_NAME, m_detectedDevice->toJson());
    qDebug() << " Detected device "
             << intToHex(m_detectedDevice->vendorId)
             << " " << intToHex(m_detectedDevice->productId);
  } else {
    qDebug() << " No device detected";
  }

  return m_detectedDevice;
}

HidDevice::~HidDevice() {
  qDebug() << "HidDevice destructor called";
  if (m_detectedDevice != nullptr) {
    delete m_detectedDevice;
  }
  if (m_hidDevice != nullptr) {
    hid_close(m_hidDevice);
  }
  const int rc = hid_exit();
  assert(rc >= 0);
}

void HidDevice::taskRunnable() {
  qDebug() << "Starting task runnable";
  unsigned char buff[8];
  while (m_detectedDevice != nullptr && m_connected) {
    int res = hid_read_timeout(m_hidDevice, buff, 8, 5000);
    qDebug() << res;
  }
}

/**
 * Transforms a number to a hexadecimal representation
 * @tparam T With number
 * @param i with value
 * @return String with hex value.
 */
template<typename T> std::string HidDevice::intToHex(T i) {
  std::stringstream stream;
  stream << "0x"
         << std::setfill('0') << std::setw(sizeof(T) * 2)
         << std::hex << i;
  return stream.str();
}