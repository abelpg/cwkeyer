#include "N1MMProxy.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/select.h>

// ── Linux: DSR monitoring via TIOCMGET polling + pipe stop signal ─────────────

N1MMProxy::N1MMProxy(IDitDah *ditDah)
    : SerialComm(false, false, false), m_ditDah(ditDah) {}

N1MMProxy::~N1MMProxy() {
  stop();
}

bool N1MMProxy::start(const std::string &portName) {
  if (!SerialComm::start(portName)) return false;

  if (pipe(m_stopPipe) != 0) {
    SerialComm::stop();
    return false;
  }
  fcntl(m_stopPipe[0], F_SETFL, O_NONBLOCK);

  m_running   = true;
  m_dsrThread = std::thread(&N1MMProxy::dsrMonitorLoop, this);
  return true;
}

void N1MMProxy::stop() {
  if (m_running) {
    m_running = false;
    if (m_stopPipe[1] >= 0) {
      char c = 1;
      (void)write(m_stopPipe[1], &c, 1);
    }
    if (m_dsrThread.joinable()) m_dsrThread.join();
    if (m_stopPipe[0] >= 0) { close(m_stopPipe[0]); m_stopPipe[0] = -1; }
    if (m_stopPipe[1] >= 0) { close(m_stopPipe[1]); m_stopPipe[1] = -1; }
  }
  SerialComm::stop();
}

void N1MMProxy::dsrMonitorLoop() {
  int lastDsr = -1; // -1 = unknown initial state
  int pipeFd  = m_stopPipe[0];

  while (m_running) {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(pipeFd, &readfds);

    struct timeval tv{ 0, 10'000 }; // 10 ms poll interval
    int ret = select(pipeFd + 1, &readfds, nullptr, nullptr, &tv);
    if (ret > 0 && FD_ISSET(pipeFd, &readfds)) break; // stop signal
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

