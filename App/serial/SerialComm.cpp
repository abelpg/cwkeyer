#include "SerialComm.h"

#ifndef _WIN32
#  include <fcntl.h>
#  include <unistd.h>
#  include <sys/ioctl.h>
#  include <termios.h>
#  include <cstring>
#  include <cerrno>
#endif

SerialComm::SerialComm(bool rtsControl, bool dtrControl, bool overlapped)
    : m_rtsControl(rtsControl), m_dtrControl(dtrControl), m_overlapped(overlapped) {
  // Nothing
}

SerialComm::~SerialComm() {
  stop();
}

bool SerialComm::start(const std::string &portName) {
  if (m_started) return false;

#ifdef _WIN32
  if (m_hSerial != INVALID_HANDLE_VALUE) stop();

  std::string fullPort = "\\\\.\\" + portName;
  m_hSerial = CreateFileA(
      fullPort.c_str(),
      GENERIC_READ | GENERIC_WRITE,
      0, nullptr,
      OPEN_EXISTING,
      m_overlapped ? FILE_FLAG_OVERLAPPED : FILE_ATTRIBUTE_NORMAL,
      nullptr
  );

  if (m_hSerial == INVALID_HANDLE_VALUE) {
    std::cerr << "SerialComm: port can not be open " << portName
              << " - error: " << GetLastError() << "\n";
    return false;
  }

  DCB dcb = {};
  dcb.DCBlength = sizeof(dcb);
  if (!GetCommState(m_hSerial, &dcb)) {
    std::cerr << "SerialComm: GetCommState fail\n";
    CloseHandle(m_hSerial);
    m_hSerial = INVALID_HANDLE_VALUE;
    return false;
  }

  dcb.BaudRate     = DEFAULT_BAUD_RATE;
  dcb.ByteSize     = 8;
  dcb.StopBits     = ONESTOPBIT;
  dcb.Parity       = NOPARITY;
  dcb.fOutxCtsFlow = m_rtsControl ? TRUE : FALSE;
  dcb.fRtsControl  = m_rtsControl ? RTS_CONTROL_ENABLE : RTS_CONTROL_DISABLE;
  dcb.fOutX        = FALSE;
  dcb.fInX         = FALSE;
  dcb.fDtrControl  = m_dtrControl ? DTR_CONTROL_ENABLE : DTR_CONTROL_DISABLE;

  if (!SetCommState(m_hSerial, &dcb)) {
    std::cerr << "SerialComm: SetCommState failed\n";
    CloseHandle(m_hSerial);
    m_hSerial = INVALID_HANDLE_VALUE;
    return false;
  }

  EscapeCommFunction(m_hSerial, CLRDTR);
  log(L_DEBUG) << "SerialComm: port opened: " << portName << "\n";
  m_started = true;
  return true;

#else // Linux / POSIX
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
  m_started = true;
  return true;
#endif
}

void SerialComm::stop() {
#ifdef _WIN32
  if (m_hSerial != INVALID_HANDLE_VALUE && m_started) {
    EscapeCommFunction(m_hSerial, CLRRTS);
    CloseHandle(m_hSerial);
    m_hSerial = INVALID_HANDLE_VALUE;
    m_started = false;
    log(L_DEBUG) << "SerialComm: port closed.\n";
  }
#else
  if (m_hSerial >= 0 && m_started) {
    int status = 0;
    ioctl(m_hSerial, TIOCMGET, &status);
    status &= ~(TIOCM_RTS | TIOCM_DTR);
    ioctl(m_hSerial, TIOCMSET, &status);
    ::close(m_hSerial);
    m_hSerial = -1;
    m_started = false;
    log(L_DEBUG) << "SerialComm: port closed.\n";
  }
#endif
}

void SerialComm::startRunCw() {
  if (!m_started) return;
#ifdef _WIN32
  if (m_hSerial == INVALID_HANDLE_VALUE) {
    std::cerr << "SerialComm::startRunCw: port closed\n";
    return;
  }
  EscapeCommFunction(m_hSerial, SETDTR);
#else
  if (m_hSerial < 0) {
    std::cerr << "SerialComm::startRunCw: port closed\n";
    return;
  }
  int status = 0;
  ioctl(m_hSerial, TIOCMGET, &status);
  status |= TIOCM_DTR;
  ioctl(m_hSerial, TIOCMSET, &status);
#endif
}

void SerialComm::stopRunCw() {
  if (!m_started) return;
#ifdef _WIN32
  if (m_hSerial == INVALID_HANDLE_VALUE) {
    std::cerr << "SerialComm::stopRunCw: port closed\n";
    return;
  }
  EscapeCommFunction(m_hSerial, CLRDTR);
#else
  if (m_hSerial < 0) {
    std::cerr << "SerialComm::stopRunCw: port closed\n";
    return;
  }
  int status = 0;
  ioctl(m_hSerial, TIOCMGET, &status);
  status &= ~TIOCM_DTR;
  ioctl(m_hSerial, TIOCMSET, &status);
#endif
}

void SerialComm::runCW(KeyerItem item, int duration) {
  if (!m_started) return;
#ifdef _WIN32
  if (m_hSerial == INVALID_HANDLE_VALUE) {
    std::cerr << "SerialComm::runCW: port closed\n";
    return;
  }
  HANDLE hSerial = m_hSerial;
  std::thread([hSerial, duration]() {
      EscapeCommFunction(hSerial, SETDTR);
      std::this_thread::sleep_for(std::chrono::milliseconds(duration));
      EscapeCommFunction(hSerial, CLRDTR);
  }).detach();
#else
  if (m_hSerial < 0) {
    std::cerr << "SerialComm::runCW: port closed\n";
    return;
  }
  int fd = m_hSerial;
  std::thread([fd, duration]() {
      int status = 0;
      ioctl(fd, TIOCMGET, &status);
      status |= TIOCM_DTR;
      ioctl(fd, TIOCMSET, &status);
      std::this_thread::sleep_for(std::chrono::milliseconds(duration));
      ioctl(fd, TIOCMGET, &status);
      status &= ~TIOCM_DTR;
      ioctl(fd, TIOCMSET, &status);
  }).detach();
#endif
}