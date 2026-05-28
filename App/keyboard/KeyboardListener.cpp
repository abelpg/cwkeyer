#include "KeyboardListener.h"

IDitDah* KeyboardListener::s_dit_dah = nullptr;
bool KeyboardListener::_dit_pressed = false;
bool KeyboardListener::_dah_pressed = false;

KeyboardListener::KeyboardListener(IDitDah* dit_dah) {
  s_dit_dah = dit_dah;
}

KeyboardListener::~KeyboardListener() {
  unhook();
}

void KeyboardListener::setEnabled(bool enabled) {
  if (m_enabled == enabled) return;
  m_enabled = enabled;
  if (m_enabled) {
    _dit_pressed = false;
    _dah_pressed = false;
    hook();
  } else {
    unhook();
  }
}

bool KeyboardListener::isEnabled() const {
  return m_enabled;
}

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
  if (nCode == HC_ACTION && s_dit_dah) {
    auto* kb = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
    bool pressed = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);

    switch (kb->vkCode) {
      case VK_OEM_PLUS:  // +/=
      case VK_RCONTROL:
        if (pressed != _dah_pressed) {
          s_dit_dah->on_dah(pressed);
          _dah_pressed = pressed;
        }
        break;
      case VK_LCONTROL:
      case VK_OEM_1:     // ;/:
        if (pressed != _dit_pressed) {
          s_dit_dah->on_dit(pressed);
          _dit_pressed= pressed;
        }
        break;
      default:
        break;
    }
  }
  return CallNextHookEx(nullptr, nCode, wParam, lParam);
}