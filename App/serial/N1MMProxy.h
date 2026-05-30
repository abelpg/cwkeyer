//
// Created by Capitang7 on 30/05/2026.
//

#ifndef CWKEYERAPP_N1MMPROXY_H
#define CWKEYERAPP_N1MMPROXY_H

#include <thread>
#include <atomic>
#include "SerialComm.h"
#include "../utils/IDitDah.h"

class N1MMProxy : public SerialComm {

public:
  explicit N1MMProxy(IDitDah *ditDah);
  ~N1MMProxy();

  bool start(const std::string &portName) override;
  void stop() override;

private:
  void dsrMonitorLoop();

  IDitDah          *m_ditDah = nullptr;
  std::thread       m_dsrThread;
};

#endif //CWKEYERAPP_N1MMPROXY_H
