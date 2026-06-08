#ifndef CWKEYERAPP_SERIALCOMM_H
#define CWKEYERAPP_SERIALCOMM_H

#include <string>
#include <chrono>
#include <thread>
#include <iostream>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>
#ifdef _WIN32
#  include <windows.h>
#else
#  include <termios.h>
#endif
#include "../utils/IKeyerCW.h"
#include "../utils/Logger.h"

class SerialComm : public IKeyerCW {

public:
  static constexpr int DEFAULT_BAUD_RATE = 9600;

  explicit SerialComm(bool rtsControl = false, bool dtrControl = false, bool overlapped = false);
  ~SerialComm();

  virtual bool start(const std::string &portName);
  virtual void stop();

  bool started() const { return m_running; }

  void runCW(KeyerItem item, int duration) override;
  void startRunCw() override;
  void stopRunCw() override;

protected:
#ifdef _WIN32
  HANDLE            m_hSerial = INVALID_HANDLE_VALUE;
#else
  int               m_hSerial = -1;
#endif
  std::atomic<bool> m_running{false};

private:
  struct CwRequest {
    KeyerItem item;
    int       duration;
  };

  bool m_rtsControl = false;
  bool m_dtrControl = false;
  bool m_overlapped = false;
  std::thread              m_workerThread;
  std::mutex               m_queueMutex;
  std::condition_variable  m_queueCv;
  std::queue<CwRequest>    m_queue;

  void processQueue();
};

#endif //CWKEYERAPP_SERIALCOMM_H
