#include "Keyboard.h"

// ─────────────────────────────────────────────────────────────────────────────
// Platform-specific key injection helpers
// ─────────────────────────────────────────────────────────────────────────────
#ifdef _WIN32

static void sendKey(WORD vk, bool pressed) {
  INPUT input{};
  input.type           = INPUT_KEYBOARD;
  input.ki.wVk         = vk;
  input.ki.wScan       = MapVirtualKey(vk, MAPVK_VK_TO_VSC);
  input.ki.dwFlags     = KEYEVENTF_SCANCODE;
  if (!pressed) input.ki.dwFlags |= KEYEVENTF_KEYUP;
  SendInput(1, &input, sizeof(INPUT));
}

#else // Linux — uinput virtual keyboard

#include <linux/uinput.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <sys/ioctl.h>

// Linux key codes matching the Windows VK roles:
//   DIT  → KEY_SEMICOLON  (VK_OEM_1)
//   DAH  → KEY_EQUAL      (VK_OEM_PLUS)
static constexpr int LINUX_KEY_DIT = KEY_SEMICOLON;
static constexpr int LINUX_KEY_DAH = KEY_EQUAL;

static int g_uinputFd = -1;

static void uinputEmit(int fd, int type, int code, int value) {
  struct input_event ev{};
  ev.type  = static_cast<__u16>(type);
  ev.code  = static_cast<__u16>(code);
  ev.value = value;
  (void)write(fd, &ev, sizeof(ev));
}

static void setupUInput() {
  g_uinputFd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
  if (g_uinputFd < 0) return;

  ioctl(g_uinputFd, UI_SET_EVBIT,  EV_KEY);
  ioctl(g_uinputFd, UI_SET_EVBIT,  EV_SYN);
  ioctl(g_uinputFd, UI_SET_KEYBIT, LINUX_KEY_DIT);
  ioctl(g_uinputFd, UI_SET_KEYBIT, LINUX_KEY_DAH);

  struct uinput_setup usetup{};
  usetup.id.bustype = BUS_USB;
  usetup.id.vendor  = 0x1EA7;
  usetup.id.product = 0x0064;
  strncpy(usetup.name, "CwKeyer Virtual Keyboard", UINPUT_MAX_NAME_SIZE - 1);

  ioctl(g_uinputFd, UI_DEV_SETUP,  &usetup);
  ioctl(g_uinputFd, UI_DEV_CREATE);

  // Give the kernel a moment to register the device
  struct timespec ts{ 0, 80'000'000L }; // 80 ms
  nanosleep(&ts, nullptr);
}

static void teardownUInput() {
  if (g_uinputFd >= 0) {
    ioctl(g_uinputFd, UI_DEV_DESTROY);
    close(g_uinputFd);
    g_uinputFd = -1;
  }
}

static void sendKey(int keyCode, bool pressed) {
  if (g_uinputFd < 0) return;
  uinputEmit(g_uinputFd, EV_KEY, keyCode, pressed ? 1 : 0);
  uinputEmit(g_uinputFd, EV_SYN, SYN_REPORT, 0);
}

#endif // platform

// ─────────────────────────────────────────────────────────────────────────────
// Keyboard class
// ─────────────────────────────────────────────────────────────────────────────

Keyboard::Keyboard(QObject *parent) : QObject(parent) {
#ifndef _WIN32
  setupUInput();
#endif
  connect(this, &Keyboard::ditChanged, this, &Keyboard::pressDit, Qt::QueuedConnection);
  connect(this, &Keyboard::dahChanged, this, &Keyboard::pressDah, Qt::QueuedConnection);
}

Keyboard::~Keyboard() {
#ifndef _WIN32
  teardownUInput();
#endif
}

void Keyboard::onDit(bool pressed) {
  if (m_enabled) emit ditChanged(pressed);
}

void Keyboard::onDah(bool pressed) {
  if (m_enabled) emit dahChanged(pressed);
}

void Keyboard::onStraight(bool pressed) {
  // do nothing
}

void Keyboard::pressDit(bool pressed) {
#ifdef _WIN32
  sendKey(VK_OEM_1, pressed);
#else
  sendKey(LINUX_KEY_DIT, pressed);
#endif
}

void Keyboard::pressDah(bool pressed) {
#ifdef _WIN32
  sendKey(VK_OEM_PLUS, pressed);
#else
  sendKey(LINUX_KEY_DAH, pressed);
#endif
}

void Keyboard::setEnabled(bool enabled) {
  m_enabled = enabled;
}
