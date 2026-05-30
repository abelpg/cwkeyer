#include "N1MMProxy.h"

N1MMProxy::N1MMProxy(IDitDah *ditDah)
    : SerialComm(false, false, true), m_ditDah(ditDah) {}

N1MMProxy::~N1MMProxy() {
  stop();
}

bool N1MMProxy::start(const std::string &portName) {
  if (!SerialComm::start(portName)) {
    return false;
  }

  // Create the manual-reset stop event (initially non-signaled)
  m_stopEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
  if (!m_stopEvent) {
    SerialComm::stop();
    return false;
  }

  // Open port in overlapped mode requires FILE_FLAG_OVERLAPPED.
  // Since we opened without it, use SetCommMask + WaitCommEvent with
  // a dedicated OVERLAPPED whose hEvent we own.
  SetCommMask(m_hSerial, EV_DSR);

  m_running   = true;
  m_dsrThread = std::thread(&N1MMProxy::dsrMonitorLoop, this);

  return true;
}

void N1MMProxy::stop() {
  if (m_running) {
    m_running = false;
    // Signal the stop event to unblock WaitForMultipleObjects
    if (m_stopEvent) {
      SetEvent(m_stopEvent);
    }
    // Also clear the comm mask to unblock any pending WaitCommEvent
    if (m_hSerial != INVALID_HANDLE_VALUE) {
      SetCommMask(m_hSerial, 0);
    }
    if (m_dsrThread.joinable()) {
      m_dsrThread.join();
    }
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

  // Read initial DSR state
  if (GetCommModemStatus(m_hSerial, &modemStatus)) {
    lastDsr = (modemStatus & MS_DSR_ON) != 0;
  }

  while (m_running) {
    // Set up an OVERLAPPED with its own auto-reset event
    OVERLAPPED ov   = {};
    ov.hEvent       = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    if (!ov.hEvent) break;

    DWORD evtMask = 0;
    // Start async WaitCommEvent
    if (!WaitCommEvent(m_hSerial, &evtMask, &ov)) {
      if (GetLastError() != ERROR_IO_PENDING) {
        CloseHandle(ov.hEvent);
        break;
      }
      // Wait for either the comm event or the stop signal
      HANDLE handles[2] = { ov.hEvent, m_stopEvent };
      DWORD  waitResult = WaitForMultipleObjects(2, handles, FALSE, INFINITE);

      if (waitResult != WAIT_OBJECT_0) {
        // Stop event signaled or error — cancel pending IO and exit
        CancelIoEx(m_hSerial, &ov);
        WaitForSingleObject(ov.hEvent, INFINITE); // let the cancel complete
        CloseHandle(ov.hEvent);
        break;
      }

      // Retrieve the result of the completed WaitCommEvent
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
          if (m_ditDah) {
            m_ditDah->onStraight(dsr);
          }
        }
      }
    }
  }
}
