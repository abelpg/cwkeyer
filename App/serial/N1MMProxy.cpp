#include "N1MMProxy.h"

#ifndef _WIN32
#  include <unistd.h>
#  include <fcntl.h>
#  include <sys/ioctl.h>
#  include <sys/select.h>
#  include <chrono>
#  include <thread>
#endif

N1MMProxy::N1MMProxy(IDitDah *ditDah)
    : SerialComm(false, false,
#ifdef _WIN32
                 true   // overlapped on Windows
#else
                 false
#endif
                 ),
      m_ditDah(ditDah) {}

N1MMProxy::~N1MMProxy() {
  stop();
}

bool N1MMProxy::start(const std::string &portName) {
  if (!SerialComm::start(portName)) {
    return false;
  }

#ifdef _WIN32
  // Create the manual-reset stop event (initially non-signaled)
  m_stopEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
  if (!m_stopEvent) {
    SerialComm::stop();
    return false;
  }

  // Monitor DSR line changes via overlapped WaitCommEvent
  SetCommMask(m_hSerial, EV_DSR);

#else
  // Create a pipe to signal the monitor thread to stop
  if (pipe(m_stopPipe) != 0) {
    SerialComm::stop();
    return false;
  }
  // Make read end non-blocking for safety
  fcntl(m_stopPipe[0], F_SETFL, O_NONBLOCK);
#endif

  m_running   = true;
  m_dsrThread = std::thread(&N1MMProxy::dsrMonitorLoop, this);

  return true;
}

void N1MMProxy::stop() {
  if (m_running) {
    m_running = false;

#ifdef _WIN32
    if (m_stopEvent) SetEvent(m_stopEvent);
    if (m_hSerial != INVALID_HANDLE_VALUE) SetCommMask(m_hSerial, 0);
#else
    // Wake up the monitoring thread by writing to the pipe
    if (m_stopPipe[1] >= 0) {
      char c = 1;
      (void)write(m_stopPipe[1], &c, 1);
    }
#endif

    if (m_dsrThread.joinable()) m_dsrThread.join();

#ifdef _WIN32
    if (m_stopEvent) {
      CloseHandle(m_stopEvent);
      m_stopEvent = nullptr;
    }
#else
    if (m_stopPipe[0] >= 0) { close(m_stopPipe[0]); m_stopPipe[0] = -1; }
    if (m_stopPipe[1] >= 0) { close(m_stopPipe[1]); m_stopPipe[1] = -1; }
#endif
  }
  SerialComm::stop();
}

#ifdef _WIN32

void N1MMProxy::dsrMonitorLoop() {
  DWORD modemStatus = 0;
  bool  lastDsr     = false;

  if (GetCommModemStatus(m_hSerial, &modemStatus))
    lastDsr = (modemStatus & MS_DSR_ON) != 0;

  while (m_running) {
    OVERLAPPED ov = {};
    ov.hEvent     = CreateEvent(nullptr, TRUE, FALSE, nullptr);
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

#else // Linux

void N1MMProxy::dsrMonitorLoop() {
  int  lastDsr = -1; // unknown
  int  pipeFd  = m_stopPipe[0];

  while (m_running) {
    // Use select() with 10 ms timeout so we can poll DSR and check the stop pipe
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(pipeFd, &readfds);

    struct timeval tv{};
    tv.tv_sec  = 0;
    tv.tv_usec = 10000; // 10 ms

    int ret = select(pipeFd + 1, &readfds, nullptr, nullptr, &tv);
    if (ret > 0 && FD_ISSET(pipeFd, &readfds)) {
      break; // stop signal received
    }

    if (m_hSerial < 0) break;

    int status = 0;
    if (ioctl(m_hSerial, TIOCMGET, &status) == 0) {
      bool dsr = (status & TIOCM_DSR) != 0;
      if (lastDsr < 0) {
        lastDsr = dsr ? 1 : 0;
      } else if (dsr != static_cast<bool>(lastDsr)) {
        lastDsr = dsr ? 1 : 0;
        if (m_ditDah) m_ditDah->onStraight(dsr);
      }
    }
  }
}

#endif
