#include "N1MMProxy.h"

N1MMProxy::N1MMProxy(IDitDah *ditDah)
    : SerialComm(false, false), m_ditDah(ditDah) {}

N1MMProxy::~N1MMProxy() {
  stop();
}

bool N1MMProxy::start(const std::string &portName) {
  if (!SerialComm::start(portName)) {
    return false;
  }

  // Enable DSR event notification
  SetCommMask(m_hSerial, EV_DSR);

  // Start DSR monitor thread
  m_running = true;
  m_dsrThread = std::thread(&N1MMProxy::dsrMonitorLoop, this);

  return true;
}

void N1MMProxy::stop() {
  if (m_running) {
    m_running = false;
    // Unblock WaitCommEvent by clearing the mask
    if (m_hSerial != INVALID_HANDLE_VALUE) {
      SetCommMask(m_hSerial, 0);
    }
    if (m_dsrThread.joinable()) {
      m_dsrThread.join();
    }
  }
  SerialComm::stop();
}

void N1MMProxy::dsrMonitorLoop() {
  DWORD evtMask      = 0;
  DWORD modemStatus  = 0;
  bool  lastDsr      = false;

  // Read initial DSR state
  if (GetCommModemStatus(m_hSerial, &modemStatus)) {
    lastDsr = (modemStatus & MS_DSR_ON) != 0;
  }

  while (m_running) {
    evtMask = 0;
    if (!WaitCommEvent(m_hSerial, &evtMask, nullptr)) {
      break; // handle closed or mask cleared → exit
    }
    if (!m_running) break;

    if (evtMask & EV_DSR) {
      modemStatus = 0;
      if (GetCommModemStatus(m_hSerial, &modemStatus)) {
        bool dsr = (modemStatus & MS_DSR_ON) != 0;
        if (dsr != lastDsr) {
          lastDsr = dsr;
          if (m_ditDah) {
            m_ditDah->onStraight(dsr);
          }
        }
      }
    }
  }
}
