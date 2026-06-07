#include "KeyboardListener.h"

#include<X11/Xlib.h>
#include <X11/XKBlib.h>
#include<X11/extensions/record.h>
#include <chrono>
#include <thread>
#include <atomic>
#include <array>
#include <mutex>

static constexpr int  DEVICE1_DIT              = 0x22;
static constexpr int  DEVICE2_DIT              = 0x25;
static constexpr int  DEVICE1_DAH              = 0x23;
static constexpr int  DEVICE2_DAH              = 0x6D;

static constexpr int  RELEASE_DELAY_MS = 60;

Display* m_controlDisplay = nullptr;   // para control (hook/unhook)
Display* m_dataDisplay    = nullptr;   // para recibir eventos (thread)
XRecordRange* m_pRange = nullptr;
XRecordContext m_context = 0;

static std::array<std::atomic<uint64_t>, 256> g_releaseDeadlineMs{};
static std::mutex g_stateMutex;


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

    g_releaseDeadlineMs[keyCode].store(0, std::memory_order_release);


    // Programa release diferido de ESTA tecla
    const uint64_t dl = nowMs() + RELEASE_DELAY_MS;
    g_releaseDeadlineMs[keyCode].store(dl, std::memory_order_release);

    if (keyCode == DEVICE1_DIT || keyCode == DEVICE2_DIT) self->setDitPressed(type == KeyPress);
    if (keyCode == DEVICE1_DAH || keyCode == DEVICE2_DAH) self->setDahPressed(type == KeyPress);


    if (type == KeyPress) {
      // Programa release diferido de ESTA tecla
      const uint64_t dl = nowMs() + RELEASE_DELAY_MS;
      g_releaseDeadlineMs[keyCode].store(dl, std::memory_order_release);
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

  m_pRange->device_events = XRecordRange8{KeyPress, KeyRelease};
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

   // XRecordEnableContext(m_dataDisplay, m_context, handle_event, reinterpret_cast<XPointer>(this));
    while (m_running) {
      XRecordProcessReplies (m_dataDisplay);

      const uint64_t t = nowMs();
      auto tryRelease = [&](unsigned char kc, bool isDit) {
        uint64_t dl = g_releaseDeadlineMs[kc].load(std::memory_order_acquire);
        if (dl != 0 && t >= dl) {
          // Consume deadline una sola vez
          if (g_releaseDeadlineMs[kc].compare_exchange_strong(dl, 0, std::memory_order_acq_rel)) {
            if (isDit) this->setDitPressed(false);
            else       this->setDahPressed(false);
          }
        }
      };

      tryRelease(DEVICE1_DIT, true);
      tryRelease(DEVICE2_DIT, true);
      tryRelease(DEVICE1_DAH, false);
      tryRelease(DEVICE2_DAH, false);

      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      // if (m_dataDisplay) {
      //   XFlush(m_dataDisplay);
      // }
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
