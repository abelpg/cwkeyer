
#ifndef CWKEYERAPP_KEYBOARDLISTENER_H
#define CWKEYERAPP_KEYBOARDLISTENER_H

#include "../utils/IDitDah.h"
#include <iostream>
#include <thread>
#include <atomic>
#include <vector>

#ifdef _WIN32
#  include <windows.h>
#endif

class KeyboardListener {
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
#else
  void readLoop();
  std::thread            m_readThread;
  std::atomic<bool>      m_running{false};
  int                    m_stopPipe[2] = {-1, -1};
  std::vector<int>       m_deviceFds;
#endif

  static IDitDah *s_ditDah;
  static bool     s_ditPressed;
  static bool     s_dahPressed;

  bool m_enabled = false;
};

#endif //CWKEYERAPP_KEYBOARDLISTENER_H
