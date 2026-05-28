#include "GuiConnector.h"



GuiConnector::GuiConnector(QApplication* app, QObject *parent) : QObject(parent) {
  load_audio_devices();
  load_comm_ports() ;
  load_configuration();

  this->app = app;

  sound = new Sound(parent);
  sound->init(m_frequency, DEFAULT_SAMPLE_RATE, m_amplitude, DEFAULT_ATTACK, DEFAULT_RELEASE);
  serialComm = new SerialComm();
  keyer = new Keyer(sound);
  keyer->init_keyer(m_wpm, static_cast<Mode>(m_mode));

  keyboard = new Keyboard(this);
  device = new UsbDevice(keyer);
  device->add_dit_dah(keyboard);

}



void GuiConnector::load_configuration() {
  // Amplitude: valor en [0.0, 1.0]
  double amp = Configuration::getValueDouble(CFG_AMPLITUDE);
  if (amp > 0.0) {
    m_amplitude = amp;
  } else {
    m_amplitude = DEFAULT_AMPLITUDE;
    Configuration::putValueDouble(CFG_AMPLITUDE, m_amplitude);
  }

  // Frequency
  double freq = Configuration::getValueDouble(CFG_FREQUENCY);
  if (freq > 0.0) {
    m_frequency = freq;
  } else {
    m_frequency = DEFAULT_FREQUENCY;
    Configuration::putValueDouble(CFG_FREQUENCY, m_frequency);
  }

  // WPM
  int wpm = Configuration::getValueInt(CFG_WPM);
  if (wpm > 0) {
    m_wpm = wpm;
  }else {
    m_wpm = DEFAULT_WPM;
    Configuration::putValueDouble(CFG_WPM, m_wpm);
  }


  int mode = Configuration::getValueInt(CFG_MODE);
  if (mode == ULTIMATIC || mode == IAMBIC_A || mode == IAMBIC_B) {
    m_mode = mode;
  } else {
    m_mode = DEFAULT_MODE;
    Configuration::putValueInt(CFG_MODE, m_mode);
  }


  // Dispositivo de audio seleccionado
  int selDev = Configuration::getValueInt(CFG_SELECTED_AUDIO_DEVICE);
  if (selDev >= 0 && selDev < m_audioDeviceList.size()) {
    m_selectedAudioDevice = selDev;
  }
}


void GuiConnector::quit() {
  qDebug() << "Quit called";
  sound->stop();
  device->disconnect_device();
  serialComm->stop();

  delete keyer;
  delete device;
  delete sound;
  delete serialComm;
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

  jsonObject["connected"] = device->connected();

  varData <<QJsonDocument(jsonObject).toJson().toStdString().c_str();

  emit device_updated(varData);

}

void GuiConnector::setAmplitude(double value) {
  if (qFuzzyCompare(m_amplitude, value)) {
    return;
  }

  m_amplitude = value;
  Configuration::putValueDouble(CFG_AMPLITUDE, m_amplitude);
  emit amplitudeChanged(m_amplitude);
  reinit_sound();
}

void GuiConnector::setFrequency(double value) {
  if (qFuzzyCompare(m_frequency, value)) {
    return;
  }
  m_frequency = value;
  emit frequencyChanged(m_frequency);
  Configuration::putValueDouble(CFG_FREQUENCY, m_frequency);
  reinit_sound();
}

void GuiConnector::setWpm(int value) {
  if (m_wpm == value) {
    return;
  }
  m_wpm = value;
  emit wpmChanged(m_wpm);
  Configuration::putValueInt(CFG_WPM, m_wpm);
  reinit_keyer();
}

void GuiConnector::setMode(int value) {
  if (m_mode == value) return;
  m_mode = value;
  emit modeChanged(m_mode);
  Configuration::putValueInt(CFG_MODE, m_mode);
  reinit_keyer();
}


void GuiConnector::setSelectedAudioDevice(int index) {
  if (index < 0 || index >= m_audioDeviceList.size()) {
    return;
  }
  if (m_selectedAudioDevice == index) {
    return;
  }
  m_selectedAudioDevice = index;
  emit selectedAudioDeviceChanged(m_selectedAudioDevice);
  Configuration::putValueInt(CFG_SELECTED_AUDIO_DEVICE, m_selectedAudioDevice);
  reinit_sound();
}

bool GuiConnector::enabledSound() const {
  return sound->enabled();
}

void GuiConnector::setEnabledSound(bool enabled) {
  sound->setEnabled(enabled);
  emit soundEnabledChanged(enabled);
}

bool GuiConnector::enabledCommOut() const {
  return serialComm->started();
}

void GuiConnector::setEnabledCommOut(bool enabled) {
  if (enabled) {
    serialComm->start(m_commPorts[m_selectedCommPort].toStdString());
  } else {
    serialComm->stop();
  }
  emit enabledCommOutChanged(enabled);
}

bool GuiConnector::enabledKeyboard() const {
  return keyboard->enabled();
}

void GuiConnector::setEnabledKeyboard(bool enabled) {
  keyboard->setEnabled(enabled);
  emit enabledKeyboardChanged(enabled);
}


void GuiConnector::load_audio_devices() {
  m_audioDeviceList = QMediaDevices::audioOutputs();
  m_audioDevices.clear();
  for (const QAudioDevice &dev : m_audioDeviceList) {
    m_audioDevices << dev.description();
  }
  emit audioDevicesChanged(m_audioDevices);
}

void GuiConnector::load_comm_ports() {

  m_commPorts = QStringList();
  for (const std::string& port : serialComm->list_ports() ) {
    m_commPorts << QString::fromStdString(port);
  }
  emit commPortsChanged(m_commPorts);
}

void GuiConnector::setSelectedCommPort(int index) {
  if (index < 0 || index >= m_commPorts.size()) {
    return;
  }
  if (m_selectedCommPort == index) {
    return;
  }

  m_selectedCommPort = index;
  emit selectedCommPortChanged(m_selectedCommPort);

}

void GuiConnector::reinit_sound() {
  sound->stop();
  if (m_selectedAudioDevice >= 0 && m_selectedAudioDevice < m_audioDeviceList.size()) {
    sound->initWithDevice(m_audioDeviceList[m_selectedAudioDevice], m_frequency, DEFAULT_SAMPLE_RATE, m_amplitude, DEFAULT_ATTACK, DEFAULT_RELEASE);
  } else {
    sound->init(m_frequency, DEFAULT_SAMPLE_RATE, m_amplitude, DEFAULT_ATTACK, DEFAULT_RELEASE);
  }
}

void GuiConnector::reinit_keyer() {
  keyer->init_keyer(m_wpm, static_cast<Mode>(m_mode));
}