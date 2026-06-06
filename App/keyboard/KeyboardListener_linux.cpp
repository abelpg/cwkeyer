#include "KeyboardListener.h"

#include <X11/Xlib.h>
#include <X11/extensions/record.h>
#include <chrono>

#define CHECK(EVENT) if (*pDatum == EVENT) std::cout << #EVENT

Display* m_controlDisplay = nullptr;
XRecordRange* m_pRange = nullptr;
XRecordContext m_context = 0;

void handle_event(XPointer, XRecordInterceptData* pRecord) {
  using XRecordDatum = char;

  log(L_DEBUG) << "handle " << pRecord->category << "---" << pRecord->data;
  // if (auto* const pDatum = reinterpret_cast<XRecordDatum*>(pRecord->data)) {
  //   CHECK(KeyPress);
  //   else CHECK(KeyRelease);
  //   else CHECK(ButtonPress);
  //   else CHECK(ButtonRelease);
  // }

  ::XRecordFreeData(pRecord);
}

void KeyboardListener::hook() {
  log(L_DEBUG) << "KeyboardListener::hook() called";
  if (m_context != 0) return;

  m_controlDisplay = XOpenDisplay(nullptr);
  if (!m_controlDisplay) { /* cleanup + return */ }

  XRecordClientSpec clients = XRecordAllClients;
  m_pRange = XRecordAllocRange();
  if (!m_pRange) { /* cleanup + return */ }

  m_pRange->device_events.first = KeyPress;
  m_pRange->device_events.last  = KeyRelease; // o ButtonRelease si quieres mouse
  m_context = XRecordCreateContext(m_controlDisplay, 0, &clients, 1, &m_pRange, 1);
  if (m_context == 0) { /* cleanup + return */ }

  m_running = true;
  m_eventThread = std::thread([this]() {
    // Bloquea y llama handle_event repetidamente
    ::XRecordEnableContext(m_controlDisplay, m_context, handle_event, nullptr);
    // Sale cuando deshabilitas el contexto
  });

  log(L_DEBUG) << "KeyboardListener::hook() called end";
}

void KeyboardListener::unhook() {
  log(L_DEBUG) << "KeyboardListener::unhook() called";

  m_running = false;

  if (m_controlDisplay && m_context) {
    ::XRecordDisableContext(m_controlDisplay, m_context);
    ::XFlush(m_controlDisplay);
  }

  if (m_eventThread.joinable()) {
    m_eventThread.join();
  }

  if (m_controlDisplay && m_context) {
    ::XRecordFreeContext(m_controlDisplay, m_context);
    m_context = 0;
  }

  if (m_pRange) {
    ::XFree(m_pRange);
    m_pRange = nullptr;
  }

  if (m_controlDisplay) {
    ::XCloseDisplay(m_controlDisplay);
    m_controlDisplay = nullptr;
  }
}
