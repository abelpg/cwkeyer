
#ifndef CWKEYERAPP_KEYBOARDLISTENER_H
#define CWKEYERAPP_KEYBOARDLISTENER_H

#include "../utils/IDitDah.h"
#include <QAbstractNativeEventFilter>
#include <QByteArray>

#include <iostream>
#include <thread>
#include <atomic>
#include <vector>

#include "../Utils/Logger.h"

#ifdef _WIN32
  #include <windows.h>
#endif

class KeyboardListener : public QAbstractNativeEventFilter {

  public:
    explicit KeyboardListener(IDitDah *ditDah);
    ~KeyboardListener();

    void setEnabled(bool enabled);
    bool isEnabled() const;
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *) override;

  private:
    void hook();
    void unhook();

#ifdef _WIN32
  static LRESULT CALLBACK lowLevelKeyboard(int nCode, WPARAM wParam, LPARAM lParam);
  HHOOK m_hook = nullptr;
  static DWORD m_dah_key;
  static DWORD m_dit_key;
#else
  std::thread m_eventThread;
  std::atomic<bool> m_running{false};
#endif

  static IDitDah *s_ditDah;
  static bool     s_ditPressed;
  static bool     s_dahPressed;

  bool m_enabled = false;
};

#endif //CWKEYERAPP_KEYBOARDLISTENER_H
