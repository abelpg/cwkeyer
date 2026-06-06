#include "KeyboardListener.h"

#include<X11/Xlib.h>
#include <X11/XKBlib.h>
#include<X11/extensions/record.h>
#include <chrono>

static constexpr int  DEVICE1_DIT              = 0x22;
static constexpr int  DEVICE2_DIT              = 0x25;
static constexpr int  DEVICE1_DAH              = 0x23;
static constexpr int  DEVICE2_DAH              = 0x6D;


std::atomic<bool> KeyboardListener::s_ditReleaseCancelled{false};
std::atomic<bool> KeyboardListener::s_dahReleaseCancelled{false};

Display* m_controlDisplay = nullptr;   // para control (hook/unhook)
Display* m_dataDisplay    = nullptr;   // para recibir eventos (thread)
XRecordRange* m_pRange = nullptr;
XRecordContext m_context = 0;


void log_key(std::string name, char type, char keyCode) {

  log(L_DEBUG) << name  << type << " keycode=0x"
               << std::hex << std::setw(2) << std::setfill('0')
               << static_cast<int>(keyCode);
}

void handle_event(XPointer closure, XRecordInterceptData* pRecord) {
  KeyboardListener* self = reinterpret_cast<KeyboardListener*>(closure);

  if (pRecord->category == XRecordFromServer && pRecord->data_len > 0) {
    const unsigned char type    = pRecord->data[0];
    const unsigned char keyCode = pRecord->data[1];

    log_key("Current: ", type, keyCode);


    // const unsigned char nextType    = pRecord->data[32];  // siguiente evento
    // const unsigned char nextKeyCode = pRecord->data[33];
    // log_key("Next: ", type, keyCode);


    if (type == KeyPress || type == KeyRelease) {
      const bool pressed = (type == KeyPress);
      if (keyCode == DEVICE1_DIT || keyCode == DEVICE2_DIT) {
        self->setDitPressed(pressed);
      } else if (keyCode == DEVICE1_DAH || keyCode == DEVICE2_DAH) {
        self->setDahPressed(pressed);
      }
    }

  }

  XRecordFreeData(pRecord);
}

void KeyboardListener::hook() {
  log(L_DEBUG) << "KeyboardListener::hook() called";
  if (m_context != 0) return;

  m_controlDisplay = XOpenDisplay(nullptr);  // display de control
  m_dataDisplay    = XOpenDisplay(nullptr);  // display de datos (separado)
  if (!m_controlDisplay || !m_dataDisplay) { /* cleanup + return */ }


  XRecordClientSpec clients = XRecordAllClients;
  m_pRange = XRecordAllocRange();
  if (!m_pRange) {
    return;
    /* cleanup + return */
  }

  m_pRange->device_events = XRecordRange8{Timer, KeyRelease};
  m_context = XRecordCreateContext(m_controlDisplay, 0, &clients, 1, &m_pRange, 1);
  if (m_context == 0) {
    return;
    /* cleanup + return */
  }

  //XSynchronize(m_controlDisplay,False);
  XSync(m_controlDisplay, False);   // <-- asegura que el contexto llegó al servidor

  if (!XRecordEnableContextAsync(m_dataDisplay, m_context, handle_event, reinterpret_cast<XPointer>(this))) {
    /* cleanup + return */
    log(L_DEBUG) << "XRecordEnableContextAsync() EXIT";
    return;
  }

  m_running = true;
  m_eventThread = std::thread([this]() {
     log(L_DEBUG) << "m_running start";
     // usa m_dataDisplay para recibir, m_controlDisplay queda libre para control
    while (m_running) {
      //XRecordEnableContext(m_dataDisplay, m_context, handle_event, reinterpret_cast<XPointer>(this));
      XRecordProcessReplies (m_dataDisplay);
    }
    log(L_DEBUG) << "m_running stop";
  });
  log(L_DEBUG) << "KeyboardListener::hook() called end";
}

void KeyboardListener::unhook() {
  log(L_DEBUG) << "KeyboardListener::unhook() called";

  m_running = false;

  if (m_controlDisplay && m_context) {
    XRecordDisableContext(m_controlDisplay, m_context);  // usa el display de control
    XFlush(m_controlDisplay);
  }

  if (m_eventThread.joinable())
    m_eventThread.join();

  if (m_controlDisplay && m_context) {
    XRecordFreeContext(m_controlDisplay, m_context);
    m_context = 0;
  }

  if (m_pRange) { XFree(m_pRange); m_pRange = nullptr; }
  if (m_dataDisplay)    { XCloseDisplay(m_dataDisplay);    m_dataDisplay    = nullptr; }
  if (m_controlDisplay) { XCloseDisplay(m_controlDisplay); m_controlDisplay = nullptr; }
  log(L_DEBUG) << "KeyboardListener::unhook() called end";
}
