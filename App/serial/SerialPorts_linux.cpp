#include "SerialPorts.h"
#include <dirent.h>
#include <sys/stat.h>
#include <algorithm>
#include <string>

// ── Linux: enumerate serial ports from /dev ───────────────────────────────────

static bool isSerialDevice(const std::string &name) {
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

