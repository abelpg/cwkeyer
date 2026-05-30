#include "SerialComm.h"

SerialComm::SerialComm(bool rtsControl, bool dtrControl, bool overlapped)
    : m_rtsControl(rtsControl), m_dtrControl(dtrControl), m_overlapped(overlapped) {
  // Nothing
}


SerialComm::~SerialComm() {
  stop();
}

bool SerialComm::start(const std::string &portName) {
  if (!m_started) {
    if (m_hSerial != INVALID_HANDLE_VALUE) {
      stop();
    }

    std::string fullPort = "\\\\.\\" + portName;

    m_hSerial = CreateFileA(
        fullPort.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0, nullptr,
        OPEN_EXISTING,
        m_overlapped? FILE_FLAG_OVERLAPPED: FILE_ATTRIBUTE_NORMAL,
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
    std::cout << "SerialComm: port opened: " << portName << "\n";
    m_started = true;
    return true;
  }
  return false;
}

void SerialComm::stop() {
  if (m_hSerial != INVALID_HANDLE_VALUE && m_started) {
    EscapeCommFunction(m_hSerial, CLRRTS);
    CloseHandle(m_hSerial);
    m_hSerial = INVALID_HANDLE_VALUE;
    m_started = false;
    std::cout << "SerialComm: port closed.\n";
  }
}

void SerialComm::startRunCw() {
  if (!m_started) return;
  if (m_hSerial == INVALID_HANDLE_VALUE) {
    std::cerr << "SerialComm::runCW: port closed\n";
    return;
  }
  EscapeCommFunction(m_hSerial, SETDTR);
}

void SerialComm::stopRunCw() {
  if (!m_started) return;
  if (m_hSerial == INVALID_HANDLE_VALUE) {
    std::cerr << "SerialComm::runCW: port closed\n";
    return;
  }
  EscapeCommFunction(m_hSerial, CLRDTR);
}

void SerialComm::runCW(KeyerItem item, int duration) {
  if (!m_started) return;
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
}