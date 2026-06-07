
#ifndef CWKEYERAPP_KEYBOARDLISTENER_H
#define CWKEYERAPP_KEYBOARDLISTENER_H

#include "../utils/IDitDah.h"
#include <QAbstractNativeEventFilter>
#include <QByteArray>

#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <vector>
#include "../Utils/Utils.h"
#include "../Utils/Logger.h"


#ifdef _WIN32
  #include <windows.h>
#endif

class KeyboardListener  {

  public:
    explicit KeyboardListener(IDitDah *ditDah);
    ~KeyboardListener();

    void setEnabled(bool enabled);
    bool isEnabled() const;
    void setDahPressed(bool pressed, int key);
    void setDitPressed(bool pressed, int key);
#ifndef _WIN32
  std::atomic<bool> m_running{false};
  
#endif

  private:
    void hook();
    void unhook();

#ifdef _WIN32
  static LRESULT CALLBACK lowLevelKeyboard(int nCode, WPARAM wParam, LPARAM lParam);
  static KeyboardListener* s_instance;  // Instancia estática singleton
  HHOOK m_hook = nullptr;
#else
  std::thread m_eventThread;
  void eventLoopWithTimer();
  void sendTimerEventToControlDisplay();
#endif

  IDitDah *s_ditDah         = nullptr;
  bool     s_ditPressed     = false;
  bool     s_dahPressed     = false;
  uint64_t s_lastDitChanged = 0;
  uint64_t s_lastDahChanged = 0;
  bool     m_enabled        = false;
  int      m_dah_key        = 0;
  int      m_dit_key        = 0;
};

#endif //CWKEYERAPP_KEYBOARDLISTENER_H
