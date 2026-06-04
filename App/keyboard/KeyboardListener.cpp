#include "KeyboardListener.h"

IDitDah *KeyboardListener::s_ditDah    = nullptr;
bool     KeyboardListener::s_ditPressed = false;
bool     KeyboardListener::s_dahPressed = false;

KeyboardListener::KeyboardListener(IDitDah *ditDah) {
  s_ditDah = ditDah;
}

KeyboardListener::~KeyboardListener() {
  unhook();
}

void KeyboardListener::setEnabled(bool enabled) {
  if (m_enabled == enabled) return;
  m_enabled = enabled;
  if (m_enabled) {
    s_ditPressed = false;
    s_dahPressed = false;
    hook();
  } else {
    unhook();
  }
}

bool KeyboardListener::isEnabled() const {
  return m_enabled;
}

// ─────────────────────────────────────────────────────────────────────────────
// Windows implementation
// ─────────────────────────────────────────────────────────────────────────────
#ifdef _WIN32

void KeyboardListener::hook() {
  if (!m_hook)
    m_hook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, nullptr, 0);
}

void KeyboardListener::unhook() {
  if (m_hook) {
    UnhookWindowsHookEx(m_hook);
    m_hook = nullptr;
  }
}

LRESULT CALLBACK KeyboardListener::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode == HC_ACTION && s_ditDah) {
    auto *kb     = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);
    bool  pressed = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);

    switch (kb->vkCode) {
      case VK_OEM_PLUS:   // +/=
      case VK_RCONTROL:
        if (pressed != s_dahPressed) {
          s_ditDah->onDah(pressed);
          s_dahPressed = pressed;
        }
        break;
      case VK_LCONTROL:
      case VK_OEM_1:      // ;/:
        if (pressed != s_ditPressed) {
          s_ditDah->onDit(pressed);
          s_ditPressed = pressed;
        }
        break;
      default:
        break;
    }
  }
  return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

// ─────────────────────────────────────────────────────────────────────────────
// Linux implementation (evdev)
// User must belong to the 'input' group, or run with appropriate privileges.
// Add the udev rule:
//   KERNEL=="event*", SUBSYSTEM=="input", GROUP="input", MODE="0660"
// ─────────────────────────────────────────────────────────────────────────────
#else

#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <cstring>
#include <string>

// Linux key codes mapped to the same roles as the Windows VK codes:
//   DIT  → KEY_SEMICOLON (;)  or KEY_LEFTCTRL
//   DAH  → KEY_EQUAL     (=)  or KEY_RIGHTCTRL
static constexpr int KEY_DIT_PRIMARY   = KEY_SEMICOLON;
static constexpr int KEY_DIT_ALT       = KEY_LEFTCTRL;
static constexpr int KEY_DAH_PRIMARY   = KEY_EQUAL;
static constexpr int KEY_DAH_ALT       = KEY_RIGHTCTRL;

// Check whether an evdev device exposes key events (i.e. is keyboard-like)
static bool hasKeyBit(int fd) {
  unsigned long evbits = 0;
  if (ioctl(fd, EVIOCGBIT(0, sizeof(evbits)), &evbits) < 0) return false;
  return (evbits >> EV_KEY) & 1;
}

void KeyboardListener::hook() {
  // Create stop pipe
  if (pipe(m_stopPipe) != 0) return;

  // Open all /dev/input/event* devices that look like keyboards
  DIR *dir = opendir("/dev/input");
  if (dir) {
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
      if (strncmp(entry->d_name, "event", 5) != 0) continue;
      std::string path = std::string("/dev/input/") + entry->d_name;
      int fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
      if (fd < 0) continue;
      if (hasKeyBit(fd)) {
        m_deviceFds.push_back(fd);
      } else {
        close(fd);
      }
    }
    closedir(dir);
  }

  m_running = true;
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

    struct timeval tv{};
    tv.tv_sec  = 0;
    tv.tv_usec = 50000; // 50 ms timeout

    int ret = select(maxFd + 1, &readfds, nullptr, nullptr, &tv);
    if (ret < 0) break;
    if (ret == 0) continue;

    // Stop pipe triggered
    if (FD_ISSET(m_stopPipe[0], &readfds)) break;

    // Read events from all ready device fds
    for (int fd : m_deviceFds) {
      if (!FD_ISSET(fd, &readfds)) continue;
      struct input_event ev{};
      while (read(fd, &ev, sizeof(ev)) == static_cast<ssize_t>(sizeof(ev))) {
        if (ev.type != EV_KEY) continue;
        bool pressed = (ev.value == 1 || ev.value == 2 /*auto-repeat*/);
        // Suppress auto-repeat: treat as held-down (same state, no transition)
        bool isRepeat = (ev.value == 2);

        if (ev.code == KEY_DIT_PRIMARY || ev.code == KEY_DIT_ALT) {
          bool newState = pressed && !isRepeat ? true : (ev.value == 0 ? false : s_ditPressed);
          if (ev.value == 0) newState = false;
          if (ev.value == 1) newState = true;
          if (newState != s_ditPressed && s_ditDah) {
            s_ditPressed = newState;
            s_ditDah->onDit(newState);
          }
        } else if (ev.code == KEY_DAH_PRIMARY || ev.code == KEY_DAH_ALT) {
          bool newState = (ev.value == 1);
          if (ev.value == 0) newState = false;
          if (ev.value == 1) newState = true;
          if (newState != s_dahPressed && s_ditDah) {
            s_dahPressed = newState;
            s_ditDah->onDah(newState);
          }
        }
      }
    }
  }
}

#endif
