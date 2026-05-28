#include "SerialComm.h"

SerialComm::SerialComm() {}

SerialComm::~SerialComm() {
  stop();
}

bool SerialComm::start(const std::string &portName) {
  if (!_started) {
    if (_hSerial != INVALID_HANDLE_VALUE) {
      stop();
    }

    // En Windows los puertos >= COM10 necesitan el prefijo \\.\

    std::string fullPort = "\\\\.\\" + portName;

    _hSerial = CreateFileA(
        fullPort.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0, nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (_hSerial == INVALID_HANDLE_VALUE) {
      std::cerr << "SerialComm: port can not be open" << portName
                << " - error: " << GetLastError() << "\n";
      return false;
    }

    DCB dcb = {};
    dcb.DCBlength = sizeof(dcb);
    if (!GetCommState(_hSerial, &dcb)) {
      std::cerr << "SerialComm: getcomstate fail\n";
      CloseHandle(_hSerial);
      _hSerial = INVALID_HANDLE_VALUE;
      return false;
    }

    dcb.BaudRate    = DEFAULT_BAUD_RATE;
    dcb.ByteSize    = 8;
    dcb.StopBits    = ONESTOPBIT;
    dcb.Parity      = NOPARITY;
    dcb.fRtsControl = RTS_CONTROL_ENABLE;

    if (!SetCommState(_hSerial, &dcb)) {
      std::cerr << "SerialComm: SetCommState falló\n";
      CloseHandle(_hSerial);
      _hSerial = INVALID_HANDLE_VALUE;
      return false;
    }

    std::cout << "SerialComm: puerto abierto: " << portName << "\n";
    _started = true;
    return true;
  }
  return false;
}

void SerialComm::stop() {
  if (_hSerial != INVALID_HANDLE_VALUE && _started) {
    EscapeCommFunction(_hSerial, CLRRTS);
    CloseHandle(_hSerial);
    _hSerial = INVALID_HANDLE_VALUE;
    _started = false;
    std::cout << "SerialComm: puerto cerrado.\n";
  }
}

std::vector<std::string> SerialComm::list_ports() {
  std::vector<std::string> ports;
  HKEY hKey;
  if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                    L"HARDWARE\\DEVICEMAP\\SERIALCOMM",
                    0, KEY_READ, &hKey) == ERROR_SUCCESS) {
    WCHAR valueName[256], data[256];
    DWORD index = 0, nameLen, dataLen, type;
    while (true) {
      nameLen = 256; dataLen = 256 * sizeof(WCHAR);
      LONG ret = RegEnumValueW(hKey, index++, valueName, &nameLen,
                               nullptr, &type,
                               reinterpret_cast<LPBYTE>(data), &dataLen);
      if (ret != ERROR_SUCCESS) break;
      // Convertir WCHAR a std::string
      int size = WideCharToMultiByte(CP_UTF8, 0, data, -1, nullptr, 0, nullptr, nullptr);
      std::string portName(size - 1, '\0');
      WideCharToMultiByte(CP_UTF8, 0, data, -1, portName.data(), size, nullptr, nullptr);
      ports.push_back(portName);
      std::cout << "Port available: " << portName << "\n";
    }
    RegCloseKey(hKey);
  }

  return ports;
}

void SerialComm::run_cw(int duration) {
  if (_hSerial == INVALID_HANDLE_VALUE || !_started) {
    std::cerr << "SerialComm::run_cw: port closed\n";
    return;
  }

  HANDLE hSerial = _hSerial;
  std::thread([hSerial, duration]() {
      EscapeCommFunction(hSerial, SETRTS);
      std::this_thread::sleep_for(std::chrono::milliseconds(duration));
      EscapeCommFunction(hSerial, CLRRTS);
  }).detach();
}