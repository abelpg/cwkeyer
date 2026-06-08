#include "Keyboard.h"
#include <linux/uinput.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <sys/ioctl.h>
#include <time.h>

// ── Linux key injection via uinput virtual keyboard ──────────────────────────
// Requires access to /dev/uinput:
//   sudo usermod -aG uinput $USER
//   or udev rule: KERNEL=="uinput", GROUP="uinput", MODE="0660"

static constexpr int LINUX_KEY_DIT = KEY_SEMICOLON; // matches VK_OEM_1
static constexpr int LINUX_KEY_DAH = KEY_EQUAL;      // matches VK_OEM_PLUS

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
  struct timespec ts{ 0, 80'000'000L };
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

// ── Keyboard class ───────────────────────────────────────────────────────────

Keyboard::Keyboard(QObject *parent) : QObject(parent) {
  setupUInput();
  connect(this, &Keyboard::ditChanged, this, &Keyboard::pressDit, Qt::QueuedConnection);
  connect(this, &Keyboard::dahChanged, this, &Keyboard::pressDah, Qt::QueuedConnection);
}

Keyboard::~Keyboard() {
  teardownUInput();
}

void Keyboard::onDit(bool pressed) {
  if (m_enabled) emit ditChanged(pressed);
}

void Keyboard::onDah(bool pressed) {
  if (m_enabled) emit dahChanged(pressed);
}

void Keyboard::onStraight(bool /*pressed*/) {}

void Keyboard::pressDit(bool pressed) {
  sendKey(LINUX_KEY_DIT, pressed);
}

void Keyboard::pressDah(bool pressed) {
  sendKey(LINUX_KEY_DAH, pressed);
}

void Keyboard::setEnabled(bool enabled) {
  m_enabled = enabled;
}

