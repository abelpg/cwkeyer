#include "SerialComm.h"
#include <QDebug>
#include <chrono>
SerialComm::SerialComm(QObject *parent) : QObject(parent) {}

SerialComm::~SerialComm() {
  stop();
}

bool SerialComm::init(const QString &portName) {
  if (_serial) {
    stop();
  }

  _serial = new QSerialPort(this);
  _serial->setPortName(portName);
  _serial->setBaudRate(DEFAULT_BAUD_RATE);
  _serial->setFlowControl(QSerialPort::NoFlowControl);  // rts_cts = false
  _serial->setParity(QSerialPort::NoParity);
  _serial->setDataBits(QSerialPort::Data8);
  _serial->setStopBits(QSerialPort::OneStop);

  if (!_serial->open(QIODevice::ReadWrite)) {
    qWarning() << "SerialComm: no se pudo abrir el puerto" << portName
               << "-" << _serial->errorString();
    delete _serial;
    _serial = nullptr;
    return false;
  }

  qDebug() << "SerialComm: puerto abierto:" << portName;
  return true;
}

void SerialComm::stop() {
  if (_serial) {
    if (_serial->isOpen()) {
      _serial->setRequestToSend(false);
      _serial->close();
    }
    delete _serial;
    _serial = nullptr;
    qDebug() << "SerialComm: puerto cerrado.";
  }
}

QStringList SerialComm::list_ports() {
  QStringList ports;
  const auto availablePorts = QSerialPortInfo::availablePorts();
  for (const QSerialPortInfo &info : availablePorts) {
    ports << info.portName();
    qDebug() << "Puerto disponible:" << info.portName()
             << "-" << info.description();
  }
  return ports;
}

void SerialComm::run_cw(int duration) {
  if (!_serial || !_serial->isOpen()) {
    qWarning() << "SerialComm::run_cw: puerto no abierto";
    return;
  }

  // Captura el puntero para usarlo en el hilo
  QSerialPort *serialPtr = _serial;

  std::thread([serialPtr, duration]() {
      serialPtr->setRequestToSend(true);
      std::this_thread::sleep_for(std::chrono::milliseconds(duration));
      serialPtr->setRequestToSend(false);
  }).detach();
}