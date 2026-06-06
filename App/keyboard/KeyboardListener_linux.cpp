#include "KeyboardListener.h"

#include <X11/Xlib.h>
#include <X11/extensions/record.h>
#include <chrono>

#define CHECK(EVENT) if (*pDatum == EVENT) std::cout << #EVENT

::Display* m_pDisplay = nullptr;
::XRecordRange* m_pRange = nullptr;
::XRecordContext m_context = 0;

void handle_event(XPointer, XRecordInterceptData* pRecord) {
  using XRecordDatum = char;

  log(L_DEBUG) << "handle " << pRecord->category << "---" << pRecord->data;
  // if (auto* const pDatum = reinterpret_cast<XRecordDatum*>(pRecord->data)) {
  //   CHECK(KeyPress);
  //   else CHECK(KeyRelease);
  //   else CHECK(ButtonPress);
  //   else CHECK(ButtonRelease);
  // }

  //::XRecordFreeData(pRecord);
}

void KeyboardListener::hook() {
  log(L_DEBUG) << "KeyboardListener::hook() called";
  if (m_pDisplay != nullptr) {
    return;
  }

  m_pDisplay = ::XOpenDisplay(nullptr);
  if (m_pDisplay == nullptr) {
    return;
  }

  XRecordClientSpec clients = XRecordAllClients;
  m_pRange = ::XRecordAllocRange();
  if (m_pRange == nullptr) {
    ::XCloseDisplay(m_pDisplay);
    m_pDisplay = nullptr;
    return;
  }

  m_pRange->device_events = XRecordRange8{KeyPress, ButtonRelease};
  m_context = ::XRecordCreateContext(m_pDisplay, 0, &clients, 1, &m_pRange, 1);
  if (m_context == 0) {
    ::XFree(m_pRange);
    m_pRange = nullptr;
    ::XCloseDisplay(m_pDisplay);
    m_pDisplay = nullptr;
    return;
  }

  ::XRecordEnableContextAsync(m_pDisplay, m_context, handle_event, nullptr); // use with/without `...Async()`
  //::XRecordProcessReplies(m_pDisplay);
  ::XFlush(m_pDisplay);
  //::XSync(m_pDisplay, true);
  log(L_DEBUG) << "KeyboardListener::hook() called end";
}

void KeyboardListener::unhook() {
  log(L_DEBUG) << "KeyboardListener::unhook() called";

  m_running = false;

  if (m_pDisplay != nullptr && m_context != 0) {
    ::XRecordDisableContext(m_pDisplay, m_context);
    ::XFlush(m_pDisplay);
  }

  if (m_eventThread.joinable()) {
    m_eventThread.join();
  }

  if (m_pDisplay != nullptr && m_context != 0) {
    ::XRecordFreeContext(m_pDisplay, m_context);
    m_context = 0;
  }

  if (m_pRange != nullptr) {
    ::XFree(m_pRange);
    m_pRange = nullptr;
  }

  if (m_pDisplay != nullptr) {
    ::XCloseDisplay(m_pDisplay);
    m_pDisplay = nullptr;
  }
}
