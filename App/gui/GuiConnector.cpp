#include "GuiConnector.h"


GuiConnector::GuiConnector(QObject *parent) : QObject(parent) {
  sound = new Sound();
  device = new HidDevice();
}

void GuiConnector::init_device() {

  emit device_initiated(QVariant());
}


void GuiConnector::connect_device() {

  emit device_connected();
}

void GuiConnector::disconnect_device() {

  emit device_disconnected();
}

void GuiConnector::detect_device() {

  QVariantList varData;
  varData.clear();

  varData <<"Detecting device....";
  emit device_updated(varData);

  varData.clear();

  Device * device_detected = this->device->detect_device();
  std::string result = "No device detected";
  if (device_detected != nullptr) {
    result = "Device vid=" + HidDevice::int_to_hex(device_detected->vendor_id) + " pid=" + HidDevice::int_to_hex(device_detected->product_id);
    varData << result.c_str();
  } else {
    varData <<"Device not detected";
  }

  emit device_updated(varData);

}

