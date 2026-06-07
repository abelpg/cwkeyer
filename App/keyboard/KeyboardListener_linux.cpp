#include "KeyboardListener.h"

#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/extensions/record.h>
#include <X11/Xatom.h>
#include <chrono>
#include <thread>
#include <atomic>
#include <array>
#include <mutex>

static constexpr int  DEVICE1_DIT              = 0x22;
static constexpr int  DEVICE2_DIT              = 0x25;
static constexpr int  DEVICE1_DAH              = 0x23;
static constexpr int  DEVICE2_DAH              = 0x6D;

Display* m_controlDisplay = nullptr;   // para control (hook/unhook)
Display* m_dataDisplay    = nullptr;   // para recibir eventos (thread)
XRecordRange* m_pRange = nullptr;
XRecordContext m_context = 0;

static constexpr int TIMER_EVENT_INTERVAL_MS = 20;


/**
 * Function to log key pressed
 * @param name String name of log
 * @param type  char type Pressed or not
 * @param keyCode char pressed
 */
void log_key(std::string name, char type, char keyCode) {

  log(L_DEBUG) << name  << (type == KeyPress?  "Pressed" : "Released")
               << " keycode=0x"
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


    if (keyCode == DEVICE1_DIT || keyCode == DEVICE2_DIT) self->setDitPressed(type == KeyPress);
    if (keyCode == DEVICE1_DAH || keyCode == DEVICE2_DAH) self->setDahPressed(type == KeyPress);

  }

  XRecordFreeData(pRecord);
}

/**
 * Generates a timer event to send to control and flush release.
 */
 void KeyboardListener::sendTimerEventToControlDisplay() {
  if (!m_controlDisplay) return;

  Window root = DefaultRootWindow(m_controlDisplay);
  Atom atom = XInternAtom(m_controlDisplay, "CWKEYER_TIMER_EVENT", False);

  XEvent ev{};
  ev.xclient.type = ClientMessage;
  ev.xclient.display = m_controlDisplay;
  ev.xclient.window = root;
  ev.xclient.message_type = atom;
  ev.xclient.format = 32;
  ev.xclient.data.l[0] = static_cast<long>(nowMs());

  XSendEvent(m_controlDisplay, root, False, SubstructureNotifyMask | SubstructureRedirectMask, &ev);
  XFlush(m_controlDisplay);
}


void KeyboardListener::eventLoopWithTimer() {
  if (!m_dataDisplay) return;

  auto nextTimerTick = std::chrono::steady_clock::now() + std::chrono::milliseconds(TIMER_EVENT_INTERVAL_MS);

  while (m_running) {
    // Call to process events.
    XRecordProcessReplies(m_dataDisplay);

    if (s_ditPressed || s_dahPressed) {
      const auto now = std::chrono::steady_clock::now();
      if (now >= nextTimerTick) {
        sendTimerEventToControlDisplay();
        nextTimerTick = now + std::chrono::milliseconds(TIMER_EVENT_INTERVAL_MS);
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }
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

  m_pRange->device_events = XRecordRange8{KeyPress, KeyRelease};
  m_context = XRecordCreateContext(m_controlDisplay, 0, &clients, 1, &m_pRange, 1);
  if (m_context == 0) {
    return;
    /* cleanup + return */
  }

  XSync(m_controlDisplay, False);   // <-- asegura que el contexto llegó al servidor

  if (!XRecordEnableContextAsync(m_dataDisplay, m_context, handle_event, reinterpret_cast<XPointer>(this))) {
    log(L_DEBUG) << "XRecordEnableContextAsync() EXIT";
    return;
  }

  m_running = true;
  m_eventThread = std::thread( &KeyboardListener::eventLoopWithTimer, this);
  m_eventThread.detach();
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
