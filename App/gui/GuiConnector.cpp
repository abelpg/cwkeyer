#include "GuiConnector.h"


GuiConnector::GuiConnector() {
  sound = new Sound();
  device = new HidDevice();
}

void GuiConnector::detect_device() {
  device->detect_device();
}

GuiConnector::~GuiConnector() {}