#include "KeyboardListener.h"
#include <windows.h>

// ── Windows global low-level keyboard hook ───────────────────────────────────

DWORD KeyboardListener::m_dah_key = 0x00;
DWORD KeyboardListener::m_dit_key = 0x00;

void KeyboardListener::hook() {
  if (!m_hook)
    m_hook = SetWindowsHookEx(WH_KEYBOARD_LL, lowLevelKeyboard, nullptr, 0);
}

void KeyboardListener::unhook() {
  if (m_hook) {
    UnhookWindowsHookEx(m_hook);
    m_hook = nullptr;
  }
}

LRESULT CALLBACK KeyboardListener::lowLevelKeyboard(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode == HC_ACTION && s_ditDah) {
    auto *kb      = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);
    bool  pressed = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
    log(L_DEBUG) << "KeyboardListener: vkCode=" << kb->vkCode << " pressed=" << pressed;
    switch (kb->vkCode) {
      case VK_OEM_PLUS:   // +/=
      case VK_RCONTROL:
        if (m_dah_key == 0 || kb->vkCode == m_dah_key) {
          m_dah_key = kb->vkCode;
          setDahPressed(pressed);
        }
        break;
      case VK_LCONTROL:
      case VK_OEM_1:      // ;/:
        if (m_dit_key == 0 || kb->vkCode == m_dit_key) {
          m_dit_key = kb->vkCode;
          setDitPressed(pressed);
        }
        break;
      default:
        break;
    }
  }
  return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

