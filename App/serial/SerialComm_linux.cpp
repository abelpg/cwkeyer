#include "SerialComm.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <cstring>
#include <cerrno>

// ── Linux implementation via POSIX termios / ioctl ────────────────────────────

SerialComm::SerialComm(bool rtsControl, bool dtrControl, bool /*overlapped*/)
    : m_rtsControl(rtsControl), m_dtrControl(dtrControl), m_overlapped(false) {}

SerialComm::~SerialComm() {
  stop();
}

bool SerialComm::start(const std::string &portName) {
  if (m_running) return false;

  m_hSerial = ::open(portName.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (m_hSerial < 0) {
    std::cerr << "SerialComm: cannot open port " << portName
              << ": " << strerror(errno) << "\n";
    return false;
  }

  struct termios tty{};
  if (tcgetattr(m_hSerial, &tty) != 0) {
    std::cerr << "SerialComm: tcgetattr failed: " << strerror(errno) << "\n";
    ::close(m_hSerial);
    m_hSerial = -1;
    return false;
  }

  cfmakeraw(&tty);
  cfsetispeed(&tty, B9600);
  cfsetospeed(&tty, B9600);
  tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
  tty.c_cflag |= CLOCAL | CREAD;
  tty.c_cflag &= ~(PARENB | PARODD);
  tty.c_cflag &= ~CSTOPB;
  tty.c_cflag &= ~CRTSCTS;

  if (tcsetattr(m_hSerial, TCSANOW, &tty) != 0) {
    std::cerr << "SerialComm: tcsetattr failed: " << strerror(errno) << "\n";
    ::close(m_hSerial);
    m_hSerial = -1;
    return false;
  }

  // Set initial DTR / RTS state
  int status = 0;
  ioctl(m_hSerial, TIOCMGET, &status);
  if (m_dtrControl) status |= TIOCM_DTR; else status &= ~TIOCM_DTR;
  if (m_rtsControl) status |= TIOCM_RTS; else status &= ~TIOCM_RTS;
  ioctl(m_hSerial, TIOCMSET, &status);

  log(L_DEBUG) << "SerialComm: port opened: " << portName << "\n";
  m_running = true;
  return true;
}

void SerialComm::stop() {
  if (m_hSerial >= 0 && m_running) {
    int status = 0;
    ioctl(m_hSerial, TIOCMGET, &status);
    status &= ~(TIOCM_RTS | TIOCM_DTR);
    ioctl(m_hSerial, TIOCMSET, &status);
    ::close(m_hSerial);
    m_hSerial = -1;
    m_running = false;
    log(L_DEBUG) << "SerialComm: port closed.\n";
  }
}

void SerialComm::startRunCw() {
  if (!m_running) return;
  if (m_hSerial < 0) { std::cerr << "SerialComm::startRunCw: port closed\n"; return; }
  int status = 0;
  ioctl(m_hSerial, TIOCMGET, &status);
  status |= TIOCM_DTR;
  ioctl(m_hSerial, TIOCMSET, &status);
}

void SerialComm::stopRunCw() {
  if (!m_running) return;
  if (m_hSerial < 0) { std::cerr << "SerialComm::stopRunCw: port closed\n"; return; }
  int status = 0;
  ioctl(m_hSerial, TIOCMGET, &status);
  status &= ~TIOCM_DTR;
  ioctl(m_hSerial, TIOCMSET, &status);
}

void SerialComm::runCW(KeyerItem /*item*/, int duration) {
  if (!m_running) return;
  if (m_hSerial < 0) { std::cerr << "SerialComm::runCW: port closed\n"; return; }
  int fd = m_hSerial;
  std::thread([fd, duration]() {
      int s = 0;
      ioctl(fd, TIOCMGET, &s); s |= TIOCM_DTR; ioctl(fd, TIOCMSET, &s);
      std::this_thread::sleep_for(std::chrono::milliseconds(duration));
      ioctl(fd, TIOCMGET, &s); s &= ~TIOCM_DTR; ioctl(fd, TIOCMSET, &s);
  }).detach();
}

