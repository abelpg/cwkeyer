
#ifndef CWKEYERAPP_KEYBOARDLISTENER_H
#define CWKEYERAPP_KEYBOARDLISTENER_H

#include "../utils/IDitDah.h"
#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include "../Utils/Logger.h"

#ifdef _WIN32
#  include <windows.h>
#else
// add in .pro file "linux: QT += x11extras" and "linux: LIBS += -lX11 -lXtst"
#include <X11/Xlib.h>
#include <X11/extensions/record.h>
#endif

class KeyboardListener  {

  public:
    explicit KeyboardListener(IDitDah *ditDah);
    ~KeyboardListener();

    void setEnabled(bool enabled);
    bool isEnabled() const;

  private:
    void hook();
    void unhook();

#ifdef _WIN32
  static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
  HHOOK m_hook = nullptr;
  static DWORD m_dah_key;
  static DWORD m_dit_key;
#else
  Display*       m_pDisplay = nullptr;
  XRecordRange*  m_pRange = nullptr;
  XRecordContext m_context = 0;
#endif

  static IDitDah *s_ditDah;
  static bool     s_ditPressed;
  static bool     s_dahPressed;

  bool m_enabled = false;
};

#endif //CWKEYERAPP_KEYBOARDLISTENER_H
