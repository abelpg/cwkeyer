/*
 * Copyright (C) 2026 EA1FXG Abel
 * This file is part of CwKeyer and is licensed under the GNU General Public License v3.0 or later.
 * See LICENSE for details.
 */
#include "KeyboardListener.h"
#include <windows.h>

// â”€â”€ Windows global low-level keyboard hook â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€


KeyboardListener* KeyboardListener::s_instance = nullptr;  // Inicializar el puntero estÃ¡tico

void KeyboardListener::hook() {
  if (!m_hook) {
    s_instance = this;  // Guardar referencia a esta instancia
    m_hook = SetWindowsHookEx(WH_KEYBOARD_LL, lowLevelKeyboard, nullptr, 0);
  }
}

void KeyboardListener::unhook() {
  if (m_hook) {
    UnhookWindowsHookEx(m_hook);
    m_hook = nullptr;
    s_instance = nullptr;
  }
}

LRESULT CALLBACK KeyboardListener::lowLevelKeyboard(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode == HC_ACTION && s_instance->s_ditDah) {
    auto *kb      = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);
    bool  pressed = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);

    switch (kb->vkCode) {
      case VK_OEM_PLUS:   // +/=
      case VK_RCONTROL:
        s_instance->setDahPressed(pressed,kb->vkCode);
        break;
      case VK_LCONTROL:
      case VK_OEM_1:      // ;/:
        s_instance->setDitPressed(pressed,kb->vkCode);
        break;
      default:
        break;
    }
  }
  return CallNextHookEx(nullptr, nCode, wParam, lParam);
}


