#include "KeyboardListener.h"
#include <windows.h>

// ── Windows global low-level keyboard hook ───────────────────────────────────

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
    auto *kb      = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);
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

