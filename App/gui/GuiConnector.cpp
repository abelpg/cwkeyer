#include "GuiConnector.h"


GuiConnector::GuiConnector(QApplication *app, QObject *parent) : QObject(parent) {
  m_app = app;

  // Create serial manager (needed before loadCommPorts)
  m_serialComm = new SerialComm();


  loadAudioDevices();
  loadCommPorts();
  loadConfiguration();

  // Create sound manager
  m_sound = new Sound(parent);
  resetSound();

  // Create CwDecoder manager
  m_cwDecoder = new CwDecoder(std::bind(&GuiConnector::onDecodeTextCw, this, std::placeholders::_1));

  // Create keyer manager
  m_keyer = new Keyer(m_sound);
  m_keyer->addKeyerCW(m_serialComm);
  m_keyer->addKeyerCW(m_cwDecoder);
  resetKeyer();

  // add N1MMProxy straight
  m_serialCommIn = new N1MMProxy(m_keyer);

  // Zadig device with keyboard sending
  m_keyboard = new Keyboard(this);
  m_device   = new UsbDevice(m_keyer);
  m_device->addDitDah(m_keyboard);

  // VBand / Vail listener if zadig device is not active
  m_keyboardListener = new KeyboardListener(m_keyer);

  log(L_DEBUG) << "GuiConnector constructor finished";
}

void GuiConnector::onDecodeTextCw(std::string text) {

  QString textToSend = QString::fromStdString(text);
  if (text == "=" || text == "<SK>" || text == "<BK>" ) {
    textToSend.append("\n");
  }

  emit textCwDecoderUpdated(textToSend);
}

void GuiConnector::loadConfiguration() {
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
  } else {
    m_wpm = DEFAULT_WPM;
    Configuration::putValueDouble(CFG_WPM, m_wpm);
  }

  int farnsWorth = Configuration::getValueInt(CFG_FARNSWORTH);
  if (farnsWorth > 0) {
    m_farnsWorth = farnsWorth;
  } else {
    m_farnsWorth = DEFAULT_FARNSWORTH;
    Configuration::putValueDouble(CFG_FARNSWORTH, m_farnsWorth);
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

  int selCom = Configuration::getValueInt(CFG_COMM_OUT);
  if (selCom >= 0 && selCom < m_commPorts.size()) {
    m_selectedCommPort = selCom;
  }

  int selCommIn = Configuration::getValueInt(CFG_COMM_IN);
  if (selCommIn >= 0 && selCommIn < m_commPorts.size()) {
    m_selectedCommPortIn = selCommIn;
  }
}

void GuiConnector::quit() {
  log(L_DEBUG) << "Quit called";
  m_sound->stop();
  m_device->disconnectDevice();
  m_serialComm->stop();
  m_serialCommIn->stop();

  delete m_keyer;
  delete m_device;
  delete m_sound;
  delete m_serialComm;
  delete m_keyboard;
}

void GuiConnector::initDevice() {
  Device *deviceConnected = m_device->initDevice();

  if (deviceConnected == nullptr) {
    m_keyboardListener->setEnabled(true);
  }

  sendDeviceUpdated(deviceConnected);
}

void GuiConnector::connectDevice() {
  Device *deviceDetected = m_device->connectDevice();
  if (deviceDetected == nullptr) {
    m_keyboardListener->setEnabled(true);
  }
  sendDeviceUpdated(deviceDetected);
}

void GuiConnector::disconnectDevice() {
  Device *deviceDetected = m_device->disconnectDevice();
  m_keyboardListener->setEnabled(false);
  sendDeviceUpdated(deviceDetected);
}

void GuiConnector::detectDevice() {
  QVariantList varData;
  varData << "Detecting device....";
  emit deviceUpdated(varData);

  varData.clear();

  Device *deviceDetected = m_device->detectDevice();
  sendDeviceUpdated(deviceDetected);
}

void GuiConnector::sendDeviceUpdated(Device *deviceDetected) {
  QVariantList varData;
  QJsonObject  jsonObject;

  if (deviceDetected != nullptr) {
    std::string text = "Device vid="
      + UsbDevice::intToHex(deviceDetected->vendorId)
      + " pid=" + UsbDevice::intToHex(deviceDetected->productId)
      + " \ninterface=" + (deviceDetected->getInterface() != nullptr
                           ? std::to_string(deviceDetected->getInterface()->interfaceNum)
                           : "N/A")
      + " endpoint=" + (deviceDetected->getInterface() != nullptr
                        ? UsbDevice::intToHex(deviceDetected->getInterface()->endpoint)
                        : "N/A");

    jsonObject["device_name"] = text.c_str();
    jsonObject["connected"]   = m_device->connected();
    emit enabledZadigChanged(m_device->connected());
  } else {
    emit enabledZadigChanged(false);
    jsonObject["device_name"] = "Vail Adapter / VBand adapter";
    jsonObject["connected"]   = m_keyboardListener->isEnabled();
  }

  varData << QJsonDocument(jsonObject).toJson().toStdString().c_str();
  emit deviceUpdated(varData);
}

void GuiConnector::setAmplitude(double value) {
  if (qFuzzyCompare(m_amplitude, value)) return;

  m_amplitude = value;
  Configuration::putValueDouble(CFG_AMPLITUDE, m_amplitude);
  emit amplitudeChanged(m_amplitude);
  resetSound();
}

void GuiConnector::setFrequency(double value) {
  if (qFuzzyCompare(m_frequency, value)) return;

  m_frequency = value;
  emit frequencyChanged(m_frequency);
  Configuration::putValueDouble(CFG_FREQUENCY, m_frequency);
  resetSound();
}

void GuiConnector::setWpm(int value) {
  if (m_wpm == value) return;

  m_wpm = value;
  emit wpmChanged(m_wpm);
  Configuration::putValueInt(CFG_WPM, m_wpm);

  if (m_farnsWorth > m_wpm) {
    setFarnsWorth(m_wpm);
  }

  resetKeyer();
}

void GuiConnector::setFarnsWorth(int value) {
  if (m_farnsWorth == value) return;

  m_farnsWorth = value;
  emit farnsWorthChanged(m_farnsWorth);
  Configuration::putValueInt(CFG_FARNSWORTH, m_farnsWorth);

  if (m_farnsWorth > m_wpm) {
    setWpm(m_farnsWorth);
  }
  resetCwDecoder();
}

void GuiConnector::setMode(int value) {
  if (m_mode == value) return;
  m_mode = value;
  emit modeChanged(m_mode);
  Configuration::putValueInt(CFG_MODE, m_mode);
  resetKeyer();
}

void GuiConnector::setSelectedAudioDevice(int index) {
  if (index < 0 || index >= m_audioDeviceList.size()) return;
  if (m_selectedAudioDevice == index) return;

  m_selectedAudioDevice = index;
  emit selectedAudioDeviceChanged(m_selectedAudioDevice);
  Configuration::putValueInt(CFG_SELECTED_AUDIO_DEVICE, m_selectedAudioDevice);
  resetSound();
}

bool GuiConnector::enabledSound() const {
  return m_sound->enabled();
}

void GuiConnector::setEnabledSound(bool enabled) {
  m_sound->setEnabled(enabled);
  emit soundEnabledChanged(enabled);
}

bool GuiConnector::enabledCommOut() const {
  return m_serialComm->started();
}

void GuiConnector::setEnabledCommOut(bool enabled) {
  if (enabled) {
    m_serialComm->start(m_commPorts[m_selectedCommPort].toStdString());
  } else {
    m_serialComm->stop();
  }
  emit enabledCommOutChanged(enabled);
}

bool GuiConnector::enabledCommIn() const {
  return m_serialCommIn->started();
}

void GuiConnector::setEnabledCommIn(bool enabled) {
  if (enabled) {
    m_serialCommIn->start(m_commPorts[m_selectedCommPortIn].toStdString());
  } else {
    m_serialCommIn->stop();
  }
  emit enabledCommInChanged(enabled);
}


bool GuiConnector::enabledKeyboard() const {
  return m_keyboard->enabled();
}

bool GuiConnector::enabledZadig() const {
  return m_device->connected();
}

void GuiConnector::setEnabledKeyboard(bool enabled) {
  if (m_keyboardListener->isEnabled() == enabled) {
    log(L_DEBUG) << "GuiConnector::setEnabledKeyboard() when listener is enabled";
    return;
  }
  m_keyboard->setEnabled(enabled);
  emit enabledKeyboardChanged(enabled);
}

bool GuiConnector::enabledCwDecoder() const {
  return m_cwDecoder->started();
}

void GuiConnector::setEnabledCwDecoder(bool enabled) {
  if (enabled) {
    m_cwDecoder->start(m_farnsWorth,m_wpm);
  } else {
    m_cwDecoder->stop();
  }
}

void GuiConnector::loadAudioDevices() {
  m_audioDeviceList = QMediaDevices::audioOutputs();
  m_audioDevices.clear();
  for (const QAudioDevice &dev : m_audioDeviceList) {
    m_audioDevices << dev.description();
  }
  emit audioDevicesChanged(m_audioDevices);
}

void GuiConnector::loadCommPorts() {
  m_commPorts = QStringList();
  for (const std::string &port : SerialPorts::listPorts()) {
    m_commPorts << QString::fromStdString(port);
  }
  emit commPortsChanged(m_commPorts);
}

void GuiConnector::setSelectedCommPort(int index) {
  if (index < 0 || index >= m_commPorts.size()) return;
  if (m_selectedCommPort == index) return;

  m_selectedCommPort = index;
  Configuration::putValueInt(CFG_COMM_OUT, m_selectedCommPort);
  emit selectedCommPortChanged(m_selectedCommPort);
}

void GuiConnector::setSelectedCommPortIn(int index) {
  if (index < 0 || index >= m_commPorts.size()) return;
  if (m_selectedCommPortIn == index) return;

  m_selectedCommPortIn = index;
  Configuration::putValueInt(CFG_COMM_IN, m_selectedCommPortIn);
  emit selectedCommPortInChanged(m_selectedCommPortIn);
}

void GuiConnector::resetSound() {
  m_sound->stop();
  if (m_selectedAudioDevice >= 0 && m_selectedAudioDevice < m_audioDeviceList.size()) {
    m_sound->initWithDevice(m_audioDeviceList[m_selectedAudioDevice],
                            m_frequency, DEFAULT_SAMPLE_RATE,
                            m_amplitude, DEFAULT_ATTACK, DEFAULT_RELEASE);
  } else {
    m_sound->init(m_frequency, DEFAULT_SAMPLE_RATE, m_amplitude, DEFAULT_ATTACK, DEFAULT_RELEASE);
  }
}

void GuiConnector::resetKeyer() {
  m_keyer->initKeyer(m_wpm, static_cast<Mode>(m_mode));
  // Always reset cwDecoder
  resetCwDecoder();
}

void GuiConnector::resetCwDecoder() {
  if (m_cwDecoder->started()) {
    m_cwDecoder->stop();
    m_cwDecoder->start(m_farnsWorth, m_wpm);
  }
}