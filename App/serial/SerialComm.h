#ifndef CWKEYERAPP_SERIALCOMM_H
#define CWKEYERAPP_SERIALCOMM_H

#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <iostream>
#include <atomic>
#include <windows.h>
#include "../utils/IKeyerCW.h"

class SerialComm : public IKeyerCW {

public:
  static constexpr int DEFAULT_BAUD_RATE = 9600;

  explicit SerialComm();
  ~SerialComm();

  bool start(const std::string &portName);
  void stop();

  int started() const { return _started; }

  std::vector<std::string> list_ports();

  void run_cw(KeyerItem item, int duration) override;

private:
  HANDLE _hSerial = INVALID_HANDLE_VALUE;
  std::atomic<bool> _running{false};
  bool _started = false;
};

#endif //CWKEYERAPP_SERIALCOMM_H

