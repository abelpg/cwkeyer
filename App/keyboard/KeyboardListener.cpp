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
      case VK_OEM_PLUS:  // +/=
      case VK_RCONTROL:
        if (pressed != s_dahPressed) {
          s_ditDah->onDah(pressed);
          s_dahPressed = pressed;
        }
        break;
      case VK_LCONTROL:
      case VK_OEM_1:     // ;/:
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