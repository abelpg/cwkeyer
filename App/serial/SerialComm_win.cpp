#include "SerialComm.h"
#include <windows.h>

// ── Windows implementation via Win32 HANDLE / DCB ────────────────────────────

SerialComm::SerialComm(bool rtsControl, bool dtrControl, bool overlapped)
    : m_rtsControl(rtsControl), m_dtrControl(dtrControl), m_overlapped(overlapped) {}

SerialComm::~SerialComm() {
  stop();
}

bool SerialComm::start(const std::string &portName) {
  if (m_running) return false;
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
    std::cerr << "SerialComm: cannot open port " << portName
              << " - error: " << GetLastError() << "\n";
    return false;
  }

  DCB dcb = {};
  dcb.DCBlength = sizeof(dcb);
  if (!GetCommState(m_hSerial, &dcb)) {
    std::cerr << "SerialComm: GetCommState failed\n";
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
  m_running = true;
  m_workerThread = std::thread(&SerialComm::processQueue, this);
  return true;
}

void SerialComm::stop() {
  if (m_running) {

    m_running = false;
    m_queueCv.notify_all();

    if (m_workerThread.joinable()) {
      m_workerThread.join();
    }
  }

  if (m_hSerial != INVALID_HANDLE_VALUE) {
    EscapeCommFunction(m_hSerial, CLRRTS);
    EscapeCommFunction(m_hSerial, CLRDTR);
    CloseHandle(m_hSerial);
    m_hSerial = INVALID_HANDLE_VALUE;
    log(L_DEBUG) << "SerialComm: port closed.\n";
  }
}

void SerialComm::startRunCw() {
  if (!m_running) return;
  if (m_hSerial == INVALID_HANDLE_VALUE) {
    std::cerr << "SerialComm::startRunCw: port closed\n";
    return;
  }
  EscapeCommFunction(m_hSerial, SETDTR);
}

void SerialComm::stopRunCw() {
  if (!m_running) return;
  if (m_hSerial == INVALID_HANDLE_VALUE) {
    std::cerr << "SerialComm::stopRunCw: port closed\n";
    return;
  }
  EscapeCommFunction(m_hSerial, CLRDTR);
}

void SerialComm::runCW(KeyerItem item, int duration) {
  if (!m_running) return;
  if (m_hSerial == INVALID_HANDLE_VALUE) {
    std::cerr << "SerialComm::runCW: port closed\n";
    return;
  }

  std::lock_guard lock(m_queueMutex);
  m_queue.push({item, duration});
  m_queueCv.notify_one();
}

void SerialComm::processQueue() {
  while (m_running) {
    CwRequest request{};

      log(L_DEBUG) << "Waitt: ";
      std::unique_lock lock(m_queueMutex);
      m_queueCv.wait(lock, [this]() { return !m_running || !m_queue.empty(); });
      log(L_DEBUG) << "SerialComm::processQueue: " << request.item ;
      if (m_queue.empty()) {
        continue;
      }

      request = m_queue.front();
      m_queue.pop();


    if (m_hSerial == INVALID_HANDLE_VALUE) {
      continue;
    }

    log(L_DEBUG) << "SerialComm::processQueue: " << request.item << "\n";
    EscapeCommFunction(m_hSerial, SETDTR);
    std::this_thread::sleep_for(std::chrono::milliseconds(request.duration));
    EscapeCommFunction(m_hSerial, CLRDTR);
  }
}

