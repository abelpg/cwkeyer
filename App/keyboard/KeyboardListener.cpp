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


void KeyboardListener::setDahPressed(bool pressed) {
  if (pressed != s_dahPressed) {
    //s_ditDah->onDah(pressed);
    log(L_DEBUG) << "Dah " << pressed;
    s_dahPressed = pressed;
  }
}

void KeyboardListener::setDitPressed(bool pressed) {
  if (pressed != s_ditPressed) {
    //s_ditDah->onDit(pressed);
    log(L_DEBUG) << "Dit " << pressed;
    s_ditPressed = pressed;
  }
}
