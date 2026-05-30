
#include "SerialPorts.h"

std::vector<std::string> SerialPorts::listPorts() {
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