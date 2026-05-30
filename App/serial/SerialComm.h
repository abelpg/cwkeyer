#ifndef CWKEYERAPP_SERIALCOMM_H
#define CWKEYERAPP_SERIALCOMM_H

#include <string>
#include <chrono>
#include <thread>
#include <iostream>
#include <atomic>
#include <windows.h>
#include "../utils/IKeyerCW.h"

class SerialComm : public IKeyerCW {

public:
  static constexpr int DEFAULT_BAUD_RATE = 9600;

  explicit SerialComm(bool rtsControl = false, bool dtrControl = false);
  ~SerialComm();

  virtual bool start(const std::string &portName);
  virtual void stop();

  bool started() const { return m_started; }

  void runCW(KeyerItem item, int duration) override;
  void startRunCw() override;
  void stopRunCw() override;

protected:
  HANDLE            m_hSerial    = INVALID_HANDLE_VALUE;
  std::atomic<bool> m_running{false};

private:
  bool m_started    = false;
  bool m_rtsControl = false;
  bool m_dtrControl = false;
};

#endif //CWKEYERAPP_SERIALCOMM_H
