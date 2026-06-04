#include "SerialPorts.h"

#ifdef _WIN32

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
      log(L_DEBUG) << "Port available: " << portName << "\n";
    }
    RegCloseKey(hKey);
  }
  return ports;
}

#else // Linux / POSIX

#include <dirent.h>
#include <sys/stat.h>
#include <cstring>
#include <algorithm>

static bool isSerialDevice(const std::string &name) {
  // Accept ttyS*, ttyUSB*, ttyACM*, ttyAMA*
  return (name.rfind("ttyS",   0) == 0 ||
          name.rfind("ttyUSB", 0) == 0 ||
          name.rfind("ttyACM", 0) == 0 ||
          name.rfind("ttyAMA", 0) == 0);
}

std::vector<std::string> SerialPorts::listPorts() {
  std::vector<std::string> ports;
  DIR *dir = opendir("/dev");
  if (!dir) return ports;

  struct dirent *entry;
  while ((entry = readdir(dir)) != nullptr) {
    std::string name(entry->d_name);
    if (!isSerialDevice(name)) continue;
    std::string fullPath = "/dev/" + name;
    struct stat st{};
    if (stat(fullPath.c_str(), &st) == 0 && S_ISCHR(st.st_mode)) {
      ports.push_back(fullPath);
      log(L_DEBUG) << "Port available: " << fullPath << "\n";
    }
  }
  closedir(dir);
  std::sort(ports.begin(), ports.end());
  return ports;
}

#endif
