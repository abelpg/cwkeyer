/*
 * Copyright (C) 2026 EA1FXG Abel
 * This file is part of CwKeyer and is licensed under the GNU General Public License v3.0 or later.
 * See LICENSE for details.
 */
#include "KeyboardListener.h"


KeyboardListener::KeyboardListener(IDitDah *ditDah) {
  s_ditDah = ditDah;
}

KeyboardListener::~KeyboardListener() {
  unhook();
}

void KeyboardListener::setEnabled(bool enabled) {
  if (m_enabled == enabled) return;
  m_enabled = enabled;
  log(L_DEBUG) << " KeyboardListener :" << (enabled?"Enabled":"Disabled");
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


void KeyboardListener::setDahPressed(bool pressed, int key) {
  if (m_dah_key != 0 && m_dah_key != key) {
    return;
  }
  m_dah_key = key;

  if (pressed != s_dahPressed) {
    s_lastDahChanged = ::nowMs();
    s_dahPressed = pressed;
    s_ditDah->onDah(pressed);
  }
}

void KeyboardListener::setDitPressed(bool pressed, int key) {
  if (m_dit_key != 0 && m_dit_key != key) {
    return;
  }
  m_dit_key = key;

  if (pressed != s_ditPressed) {
    s_lastDitChanged = ::nowMs();
    s_ditPressed = pressed;
    s_ditDah->onDit(pressed);
  }
}

