#include "GuiConnector.h"


GuiConnector::GuiConnector(QObject *parent) : QObject(parent) {
  sound = new Sound();
  device = new HidDevice();
}


void GuiConnector::detect_device() {

  QVariantList varData;
  varData.clear();

  varData <<"Detecting device....";
  emit device_updated(varData);

  varData.clear();
  if (device->detect_device()) {
    varData << device->get_current_device().c_str();
  } else {
    varData <<"Device not detected";
  }

  emit device_updated(varData);

}

