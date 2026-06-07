#include "Keyer.h"


Keyer::Keyer(IKeyerCW *soundCW) {
  log(L_DEBUG) << "Keyer constructor called";
  m_mutex     = new std::mutex();
  addKeyerCW(soundCW);
}

Keyer::~Keyer() {
  stopKeyer();

  delete m_mutex;
  delete m_threadKeyer;
}

void Keyer::stopKeyer() {
  if (m_started) {
    log(L_INFO) << "Keyer started. reset thread";
    m_started = false;
    m_coditionVar.notify_all();
    if (m_threadKeyer != nullptr && m_threadKeyer->joinable()) {
      m_threadKeyer->join();
    }
    log(L_INFO) << "Keyer stopped. reset thread";
    delete m_threadKeyer;
    m_threadKeyer = nullptr;
  }
}

void Keyer::initKeyer(int wpm, Mode mode) {
  log(L_DEBUG) << "Keyer initKeyer called";
  stopKeyer();

  m_ditTime   = IKeyerCW::calculateDuration(DIT, wpm);
  m_dahTime   = IKeyerCW::calculateDuration(DAH, wpm);
  m_spaceTime = IKeyerCW::calculateDuration(INTER_ELEMENT_SPACE, wpm);
  m_mode      = mode;

  m_started = true;
  m_threadKeyer    = new std::thread(&Keyer::keyerCall, this);
  m_threadKeyer->detach();
}

void Keyer::onDit(bool pressed) {
  if (pressed) {
    m_ditPressed  = true;
    m_lastPressed = DIT;
    log(L_DEBUG) << "Enqueue DIT";
    enqueue(DIT);
  } else {
    log(L_DEBUG) << "Stop DIT";
    m_ditPressed = false;
  }
}

void Keyer::onDah(bool pressed) {
  if (pressed) {
    m_dahPressed  = true;
    m_lastPressed = DAH;
    log(L_DEBUG) << "Enqueue DAH";
    enqueue(DAH);
  } else {
    log(L_DEBUG) << "Stop DAH";
    m_dahPressed = false;
  }
}

void Keyer::onStraight(bool pressed) {
  // Send direct
  for (IKeyerCW *keyerCW : m_keyerCWList) {
    if (pressed) {
      keyerCW->startRunCw();
    } else {
      keyerCW->stopRunCw();
    }
  }
}

void Keyer::enqueue(KeyerItem item) {
  if (m_queue.size() < 1 ) {
    m_lastQueued = item;
    m_queue.push(item);
    m_coditionVar.notify_one();
  }
}

void Keyer::addKeyerCW(IKeyerCW *keyerCW) {
  m_keyerCWList.push_back(keyerCW);
}

void Keyer::playDitDah(KeyerItem item) {
  if (item == DIT) {
    for (IKeyerCW *keyerCW : m_keyerCWList) {
      keyerCW->runCW(DIT, m_ditTime);
    }
    Utils::sleepFor(m_ditTime + m_spaceTime);
  } else if (item == DAH) {
    for (IKeyerCW *keyerCW : m_keyerCWList) {
      keyerCW->runCW(DAH, m_dahTime);
    }
    Utils::sleepFor(m_dahTime + m_spaceTime);
  }
}

void Keyer::keyerCall() {
  while (m_started) {
    // Wait until queue is not empty to avoid spurious wakeups
    std::unique_lock lock(*m_mutex);
    m_coditionVar.wait(lock, [this]() { return !m_queue.empty() ||  !m_started; });

    if (m_queue.empty()) {
      // To break
      continue;
    }
    uint64_t startTime = nowMs();
    KeyerItem item = std::move(m_queue.front());
    m_queue.pop();
    const bool squeeze = m_ditPressed && m_dahPressed;
    playDitDah(item);
    log(L_DEBUG) << "After play:" << (nowMs() - startTime) << "ms";

    // Process mode
    if (squeeze) {
      if (m_mode == ULTIMATIC) {
        enqueue(m_lastPressed);
      } else if (m_mode == IAMBIC_B || m_mode == IAMBIC_A) {
        enqueue(reverse(m_lastQueued));
      }
    } else if (m_ditPressed) {
      enqueue(DIT);
    } else if (m_dahPressed) {
      enqueue(DAH);
    }

    log(L_DEBUG) << "Keyer call duration:" << (nowMs() - startTime) << "ms";

  }
  log(L_INFO) << "Keyer thread end";
}

int Keyer::ditTime()   const { return m_ditTime; }
int Keyer::dahTime()   const { return m_dahTime; }
int Keyer::spaceTime() const { return m_spaceTime; }
