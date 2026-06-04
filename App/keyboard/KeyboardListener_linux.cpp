#include "KeyboardListener.h"
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <cstring>
#include <string>

// ── Linux global keyboard listener via evdev ─────────────────────────────────
// Requires read access to /dev/input/event*:
//   sudo usermod -aG input $USER
//   or udev rule: KERNEL=="event*", SUBSYSTEM=="input", GROUP="input", MODE="0660"

// Key codes matching the Windows VK roles:
//   DIT → KEY_SEMICOLON (;)  /  KEY_LEFTCTRL
//   DAH → KEY_EQUAL     (=)  /  KEY_RIGHTCTRL
static constexpr int KEY_DIT_PRIMARY = KEY_SEMICOLON;
static constexpr int KEY_DIT_ALT     = KEY_LEFTCTRL;
static constexpr int KEY_DAH_PRIMARY = KEY_EQUAL;
static constexpr int KEY_DAH_ALT     = KEY_RIGHTCTRL;

static bool hasKeyBit(int fd) {
  unsigned long evbits = 0;
  if (ioctl(fd, EVIOCGBIT(0, sizeof(evbits)), &evbits) < 0) return false;
  return (evbits >> EV_KEY) & 1;
}

// ── hook / unhook ────────────────────────────────────────────────────────────

void KeyboardListener::hook() {
  if (pipe(m_stopPipe) != 0) return;

  DIR *dir = opendir("/dev/input");
  if (dir) {
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
      if (strncmp(entry->d_name, "event", 5) != 0) continue;
      std::string path = std::string("/dev/input/") + entry->d_name;
      int fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
      if (fd < 0) continue;
      if (hasKeyBit(fd))
        m_deviceFds.push_back(fd);
      else
        close(fd);
    }
    closedir(dir);
  }

  m_running    = true;
  m_readThread = std::thread(&KeyboardListener::readLoop, this);
}

void KeyboardListener::unhook() {
  if (m_running) {
    m_running = false;
    if (m_stopPipe[1] >= 0) {
      char c = 1;
      (void)write(m_stopPipe[1], &c, 1);
    }
    if (m_readThread.joinable()) m_readThread.join();
  }
  for (int fd : m_deviceFds) close(fd);
  m_deviceFds.clear();
  if (m_stopPipe[0] >= 0) { close(m_stopPipe[0]); m_stopPipe[0] = -1; }
  if (m_stopPipe[1] >= 0) { close(m_stopPipe[1]); m_stopPipe[1] = -1; }
}

// ── read loop (runs in dedicated thread) ────────────────────────────────────

void KeyboardListener::readLoop() {
  while (m_running) {
    fd_set readfds;
    FD_ZERO(&readfds);

    int maxFd = m_stopPipe[0];
    FD_SET(m_stopPipe[0], &readfds);
    for (int fd : m_deviceFds) {
      FD_SET(fd, &readfds);
      if (fd > maxFd) maxFd = fd;
    }

    struct timeval tv{ 0, 50'000 }; // 50 ms
    int ret = select(maxFd + 1, &readfds, nullptr, nullptr, &tv);
    if (ret < 0) break;
    if (ret == 0) continue;

    if (FD_ISSET(m_stopPipe[0], &readfds)) break;

    for (int fd : m_deviceFds) {
      if (!FD_ISSET(fd, &readfds)) continue;

      struct input_event ev{};
      while (read(fd, &ev, sizeof(ev)) == static_cast<ssize_t>(sizeof(ev))) {
        if (ev.type != EV_KEY) continue;

        // ev.value: 0=release, 1=press, 2=autorepeat
        if (ev.code == KEY_DIT_PRIMARY || ev.code == KEY_DIT_ALT) {
          bool newState = (ev.value == 1);
          if (ev.value == 2) continue; // ignore autorepeat
          if (newState != s_ditPressed && s_ditDah) {
            s_ditPressed = newState;
            s_ditDah->onDit(newState);
          }
        } else if (ev.code == KEY_DAH_PRIMARY || ev.code == KEY_DAH_ALT) {
          bool newState = (ev.value == 1);
          if (ev.value == 2) continue;
          if (newState != s_dahPressed && s_ditDah) {
            s_dahPressed = newState;
            s_ditDah->onDah(newState);
          }
        }
      }
    }
  }
}

