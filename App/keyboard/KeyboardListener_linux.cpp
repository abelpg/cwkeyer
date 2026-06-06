#include "KeyboardListener.h"

#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include <X11/extensions/record.h>


Display* m_controlDisplay = nullptr;
XRecordRange* m_pRange = nullptr;
XRecordContext m_context = 0;

void handle_event(XPointer, XRecordInterceptData* pRecord) {
  using XRecordDatum = char;

  if (pRecord->data_len > 0 ) {
    auto* const pDatum = reinterpret_cast<XRecordDatum*>(pRecord->data);
    log(L_DEBUG) << "KeyPress event detected " << pDatum;


  }

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

  m_pRange->device_events = XRecordRange8{KeyPress, KeyRelease};
  m_context = XRecordCreateContext(m_controlDisplay, 0, &clients, 1, &m_pRange, 1);
  if (m_context == 0) { /* cleanup + return */ }

  XRecordEnableContextAsync(m_controlDisplay, m_context, handle_event, nullptr); // use with/without `...Async()`

  XRecordProcessReplies(m_controlDisplay);
  XFlush(m_controlDisplay);


  m_eventThread = std::thread([this]() {
    // Bloquea y llama handle_event repetidamente
    log(L_DEBUG) << "Keyboard listener thread started";
    XSync(m_controlDisplay, true);
    log(L_DEBUG) << "Keyboard listener thread STOPED";
    // Sale cuando deshabilitas el contexto
  });

  log(L_DEBUG) << "KeyboardListener::hook() called end";
}

void KeyboardListener::unhook() {
  log(L_DEBUG) << "KeyboardListener::unhook() called";

  if (m_controlDisplay && m_context) {
    XRecordDisableContext(m_controlDisplay, m_context);
    XFlush(m_controlDisplay);
  }

  if (m_controlDisplay && m_context) {
    XRecordFreeContext(m_controlDisplay, m_context);
    m_context = 0;
  }

  if (m_pRange) {
    XFree(m_pRange);
    m_pRange = nullptr;
  }

  if (m_controlDisplay) {
    XCloseDisplay(m_controlDisplay);
    m_controlDisplay = nullptr;
  }

  if (m_eventThread.joinable()) {
    m_eventThread.join();
  }
}
