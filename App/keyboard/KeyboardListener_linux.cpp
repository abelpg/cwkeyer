#include "KeyboardListener.h"

#include<X11/Xlib.h>
#include<X11/extensions/record.h>

static constexpr int  DEVICE1_DIT              = 0x22;
static constexpr int  DEVICE2_DIT              = 0x25;
static constexpr int  DEVICE1_DAH              = 0x23;
static constexpr int  DEVICE2_DAH              = 0x6D;

Display* m_controlDisplay = nullptr;
XRecordRange* m_pRange = nullptr;
XRecordContext m_context = 0;


void handle_event(XPointer closure, XRecordInterceptData* pRecord) {
  KeyboardListener* self = reinterpret_cast<KeyboardListener*>(closure);

  if (pRecord->category == XRecordFromServer && pRecord->data_len > 0) {
    const unsigned char type    = pRecord->data[0];  // KeyPress=2, KeyRelease=3
    const unsigned char keyCode = pRecord->data[1];  // keycode de la tecla

    const char* action = (type == KeyPress) ? "KeyPress" : "KeyRelease";
    log(L_DEBUG) << action << " keycode=0x"
                 << std::hex << std::uppercase
                 << std::setw(2) << std::setfill('0')
                 << static_cast<int>(keyCode);

    if (keyCode == DEVICE1_DIT) {
      //self->s_ditDah->onDit(type == KeyPress);
      self->s_ditPressed = (type == KeyPress);
    } else if (keyCode == DEVICE2_DAH) {
      //self->s_ditDah->onDah(type == KeyPress);
      self->s_dahPressed = (type == KeyPress);
    }
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

  XRecordEnableContextAsync(m_controlDisplay, m_context, handle_event, reinterpret_cast<XPointer>(this)); // use with/without `...Async()`

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
