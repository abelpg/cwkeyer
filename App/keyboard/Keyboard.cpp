
#include "Keyboard.h"


// Helper: simulate a key press/release for a given virtual key
static void sendKey(WORD vk, bool pressed) {
  INPUT input{};
  input.type       = INPUT_KEYBOARD;
  input.ki.wVk     = vk;
  input.ki.dwFlags = pressed ? 0 : KEYEVENTF_KEYUP;
  SendInput(1, &input, sizeof(INPUT));
}

Keyboard::Keyboard(QObject *parent) : QObject(parent) {
  // Queued connections → slots run in the receiver's thread (event loop),
  // so on_dit/on_dah return immediately without blocking.
  connect(this, &Keyboard::ditChanged, this, &Keyboard::pressDit, Qt::QueuedConnection);
  connect(this, &Keyboard::dahChanged, this, &Keyboard::pressDah, Qt::QueuedConnection);
}

// Called from any thread — just emits a signal and returns immediately
void Keyboard::on_dit(bool pressed) {
  if (m_enabled) {
    emit ditChanged(pressed);
  }
}

void Keyboard::on_dah(bool pressed) {
  if (m_enabled) {
    emit dahChanged(pressed);
  }
}

// Executed by the Qt event loop (non-blocking for the caller)
void Keyboard::pressDit(bool pressed) {
  qDebug() << "Keyboard::pressDit() called with pressed=" << pressed;
  sendKey(VK_LCONTROL, pressed);   // Left Ctrl
}

void Keyboard::pressDah(bool pressed) {
  qDebug() << "Keyboard::pressDah() called with pressed=" << pressed;
  sendKey(VK_RCONTROL, pressed);   // Right Ctrl
}
void Keyboard::setEnabled(bool enabled) {
  m_enabled = enabled;
}

