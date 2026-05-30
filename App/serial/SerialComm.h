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

  bool started() const { return m_started; }

  void runCW(KeyerItem item, int duration) override;
  void startRunCw() override;
  void stopRunCw() override;

private:
  HANDLE            m_hSerial = INVALID_HANDLE_VALUE;
  std::atomic<bool> m_running{false};
  bool              m_started = false;
};

#endif //CWKEYERAPP_SERIALCOMM_H
