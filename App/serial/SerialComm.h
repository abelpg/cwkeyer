#ifndef CWKEYERAPP_SERIALCOMM_H
#define CWKEYERAPP_SERIALCOMM_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QString>
#include <QStringList>
#include <thread>
#include <atomic>
#include "../utils/IKeyerCW.h"


class SerialComm : public QObject, public IKeyerCW {
  Q_OBJECT

public:
  static constexpr int DEFAULT_BAUD_RATE = 9600;

  explicit SerialComm(QObject *parent = nullptr);
  ~SerialComm();

  // Inicializa y abre el puerto serie con los parámetros por defecto
  bool init(const QString &portName);

  // Cierra y libera el puerto serie
  void stop();

  // Lista los puertos disponibles (ej: COM1, COM2, ...)
  QStringList list_ports();

  // Override de IKeyerCW: activa RTS durante 'duration' ms en un hilo nuevo
  void run_cw(int duration) override;

private:
  QSerialPort *_serial = nullptr;
  std::atomic<bool> _running{false};
};


#endif //CWKEYERAPP_SERIALCOMM_H
