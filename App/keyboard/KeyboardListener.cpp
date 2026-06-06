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

bool KeyboardListener:: nativeEventFilter(const QByteArray &eventType, void *message, qintptr *) {
  log(L_DEBUG) << "KeyboardListener::nativeEventFilter called with eventType=" << *eventType;
  return false;
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
