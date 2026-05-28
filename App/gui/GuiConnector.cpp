#include "GuiConnector.h"

#include <QtMultimedia/QMediaDevices>
#include <QtMultimedia/QAudioDevice>

GuiConnector::GuiConnector(QObject *parent) : QObject(parent) {
  load_audio_devices();
  load_configuration();
  sound = new Sound(parent);
  sound->init(m_frequency, DEFAULT_SAMPLE_RATE, m_amplitude, DEFAULT_ATTACK, DEFAULT_RELEASE);
  keyer = new Keyer(sound);
  keyer->init_keyer(m_wpm, static_cast<Mode>(m_mode));
  device = new UsbDevice(keyer);
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

  delete keyer;
  delete device;
  delete sound;
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
  qDebug() << "GET ENABLED" << sound->enabled();
  return sound->enabled();
}

void GuiConnector::setEnabledSound(bool enabled) {
  qDebug() << enabled;
  sound->setEnabled(enabled);
  emit soundEnabledChanged(enabled);
}

void GuiConnector::load_audio_devices() {
  m_audioDeviceList = QMediaDevices::audioOutputs();
  m_audioDevices.clear();
  for (const QAudioDevice &dev : m_audioDeviceList) {
    m_audioDevices << dev.description();
  }
  emit audioDevicesChanged(m_audioDevices);
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