#include "KeyboardListener.h"
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <cstring>
#include <string>
#include <vector>

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

// EVIOCGBIT needs a buffer large enough for all event types.
// EV_MAX is the highest event type; we need ceil((EV_MAX+1)/8) bytes.
static bool hasKeyBit(int fd) {
  uint8_t evbits[(EV_MAX + 7) / 8 + 1] = {};
  if (ioctl(fd, EVIOCGBIT(0, sizeof(evbits)), evbits) < 0) return false;
  return (evbits[EV_KEY / 8] >> (EV_KEY % 8)) & 1;
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
  // Build the pollfd array: first entry is the stop pipe, rest are device fds.
  // poll() has no FD_SETSIZE limitation unlike select().
  std::vector<struct pollfd> pfds;
  pfds.reserve(m_deviceFds.size() + 1);

  // Stop pipe wakeup fd (index 0)
  pfds.push_back({ m_stopPipe[0], POLLIN, 0 });

  // One entry per evdev device
  for (int fd : m_deviceFds)
    pfds.push_back({ fd, POLLIN, 0 });

  while (m_running) {
    // Reset revents before each poll call
    for (auto &pfd : pfds) pfd.revents = 0;

    int ret = poll(pfds.data(), static_cast<nfds_t>(pfds.size()), 50 /*ms*/);
    if (ret < 0) break;   // error
    if (ret == 0) continue; // timeout

    // Check stop pipe (index 0)
    if (pfds[0].revents & POLLIN) break;

    // Check device fds (index 1 onwards)
    for (std::size_t i = 1; i < pfds.size(); ++i) {
      if (!(pfds[i].revents & POLLIN)) continue;

      struct input_event ev{};
      while (read(pfds[i].fd, &ev, sizeof(ev)) == static_cast<ssize_t>(sizeof(ev))) {
        if (ev.type != EV_KEY) continue;

        // ev.value: 0=release, 1=press, 2=autorepeat
        if (ev.code == KEY_DIT_PRIMARY || ev.code == KEY_DIT_ALT) {
          if (ev.value == 2) continue; // ignore autorepeat
          bool newState = (ev.value == 1);
          if (newState != s_ditPressed && s_ditDah) {
            s_ditPressed = newState;
            s_ditDah->onDit(newState);
          }
        } else if (ev.code == KEY_DAH_PRIMARY || ev.code == KEY_DAH_ALT) {
          if (ev.value == 2) continue;
          bool newState = (ev.value == 1);
          if (newState != s_dahPressed && s_ditDah) {
            s_dahPressed = newState;
            s_ditDah->onDah(newState);
          }
        }
      }
    }
  }
}

