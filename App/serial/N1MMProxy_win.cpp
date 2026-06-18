#include "N1MMProxy.h"
#include <windows.h>

// ── Windows: DSR monitoring via overlapped WaitCommEvent ─────────────────────

N1MMProxy::N1MMProxy(IDitDah *ditDah)
    : SerialComm(false, false, true /*overlapped*/), m_ditDah(ditDah) {}

N1MMProxy::~N1MMProxy() {
  stop();
}

bool N1MMProxy::start(const std::string &portName) {
  if (!SerialComm::start(portName)) return false;

  m_stopEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
  if (!m_stopEvent) {
    SerialComm::stop();
    return false;
  }

  SetCommMask(m_hSerial, EV_DSR);

  m_running   = true;
  m_dsrThread = std::thread(&N1MMProxy::dsrMonitorLoop, this);
  return true;
}

void N1MMProxy::stop() {
  if (m_running) {
    m_running = false;
    if (m_stopEvent)                         SetEvent(m_stopEvent);
    if (m_hSerial != INVALID_HANDLE_VALUE)   SetCommMask(m_hSerial, 0);
    if (m_dsrThread.joinable())              m_dsrThread.join();
    if (m_stopEvent) {
      CloseHandle(m_stopEvent);
      m_stopEvent = nullptr;
    }
  }
  SerialComm::stop();
}

void N1MMProxy::dsrMonitorLoop() {
  DWORD modemStatus = 0;
  bool  lastDsr     = false;

  if (GetCommModemStatus(m_hSerial, &modemStatus))
    lastDsr = (modemStatus & MS_DSR_ON) != 0;

  while (m_running) {
    OVERLAPPED ov   = {};
    ov.hEvent       = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    if (!ov.hEvent) break;

    DWORD evtMask = 0;
    if (!WaitCommEvent(m_hSerial, &evtMask, &ov)) {
      if (GetLastError() != ERROR_IO_PENDING) {
        CloseHandle(ov.hEvent);
        break;
      }
      HANDLE handles[2] = { ov.hEvent, m_stopEvent };
      DWORD  waitResult = WaitForMultipleObjects(2, handles, FALSE, INFINITE);
      if (waitResult != WAIT_OBJECT_0) {
        CancelIoEx(m_hSerial, &ov);
        WaitForSingleObject(ov.hEvent, INFINITE);
        CloseHandle(ov.hEvent);
        break;
      }
      DWORD transferred = 0;
      if (!GetOverlappedResult(m_hSerial, &ov, &transferred, FALSE)) {
        CloseHandle(ov.hEvent);
        break;
      }
    }

    CloseHandle(ov.hEvent);
    if (!m_running) break;

    if (evtMask & EV_DSR) {
      modemStatus = 0;
      if (GetCommModemStatus(m_hSerial, &modemStatus)) {
        bool dsr = (modemStatus & MS_DSR_ON) != 0;
        if (dsr != lastDsr) {
          lastDsr = dsr;
          if (m_ditDah) m_ditDah->onStraight(dsr);
        }
      }
    }
  }
}

