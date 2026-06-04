#include "Keyboard.h"
#include <windows.h>

// ── Windows key injection via SendInput ──────────────────────────────────────

static void sendKey(WORD vk, bool pressed) {
  INPUT input{};
  input.type       = INPUT_KEYBOARD;
  input.ki.wVk     = vk;
  input.ki.wScan   = MapVirtualKey(vk, MAPVK_VK_TO_VSC);
  input.ki.dwFlags = KEYEVENTF_SCANCODE;
  if (!pressed) input.ki.dwFlags |= KEYEVENTF_KEYUP;
  SendInput(1, &input, sizeof(INPUT));
}

Keyboard::Keyboard(QObject *parent) : QObject(parent) {
  connect(this, &Keyboard::ditChanged, this, &Keyboard::pressDit, Qt::QueuedConnection);
  connect(this, &Keyboard::dahChanged, this, &Keyboard::pressDah, Qt::QueuedConnection);
}

Keyboard::~Keyboard() {}

void Keyboard::onDit(bool pressed) {
  if (m_enabled) emit ditChanged(pressed);
}

void Keyboard::onDah(bool pressed) {
  if (m_enabled) emit dahChanged(pressed);
}

void Keyboard::onStraight(bool /*pressed*/) {}

void Keyboard::pressDit(bool pressed) {
  sendKey(VK_OEM_1, pressed);   // ;/:
}

void Keyboard::pressDah(bool pressed) {
  sendKey(VK_OEM_PLUS, pressed); // +/=
}

void Keyboard::setEnabled(bool enabled) {
  m_enabled = enabled;
}

