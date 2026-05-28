
#ifndef CWKEYERAPP_KEYBOARDLISTENER_H
#define CWKEYERAPP_KEYBOARDLISTENER_H

#include "../utils/IDitDah.h"
#include <windows.h>
#include <iostream>
class KeyboardListener {
public:
  explicit KeyboardListener(IDitDah* dit_dah);
  ~KeyboardListener();

  void setEnabled(bool enabled);
  bool isEnabled() const;

private:
  void hook();
  void unhook();

  static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
  static IDitDah* s_dit_dah;
  static bool _dit_pressed;
  static bool _dah_pressed;

  HHOOK m_hook = nullptr;
  bool m_enabled = false;


};

#endif //CWKEYERAPP_KEYBOARDLISTENER_H
