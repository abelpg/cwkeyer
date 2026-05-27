#include "GuiConnector.h"

GuiConnector::GuiConnector(QObject *parent) : QObject(parent) {
  sound = new Sound();
  keyer = new Keyer(25);
  device = new UsbDevice(keyer);
}

void GuiConnector::init_device() {
  Device * device_connected = this->device->init_device();
  send_device_updated(device_connected);
}


void GuiConnector::connect_device() {
  Device * device_detected = this->device->connect_device();
  send_device_updated(device_detected);
}

void GuiConnector::disconnect_device() {

  Device * device_detected = this->device->disconnect_device();
  send_device_updated(device_detected);

}

void GuiConnector::detect_device() {

  QVariantList varData;
  varData.clear();

  varData <<"Detecting device....";
  emit device_updated(varData);

  varData.clear();

  Device * device_detected = this->device->detect_device();
  send_device_updated(device_detected);

}

void GuiConnector::send_device_updated(Device * device_detected) {
  QVariantList varData;
  varData.clear();

  QJsonObject jsonObject;

  if (device_detected != nullptr) {
    std::string text = "Device vid="
      + UsbDevice::int_to_hex(device_detected->vendor_id)
      + " pid=" + UsbDevice::int_to_hex(device_detected->product_id)
      + " \ninterface=" + (device_detected->getInterface() != nullptr? std::to_string(device_detected->getInterface()->interface): "N/A")
      + " endpoint=" + (device_detected->getInterface() != nullptr? UsbDevice::int_to_hex(device_detected->getInterface()->endpoint): "N/A");

    jsonObject["device_name"] = text.c_str();
  } else {
    jsonObject["device_name"] = "Device not detected";
  }

  jsonObject["connected"] = device_detected!= nullptr? device_detected->connected:false;

  varData <<QJsonDocument(jsonObject).toJson().toStdString().c_str();

  emit device_updated(varData);

}
