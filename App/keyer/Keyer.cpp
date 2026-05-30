#include "Keyer.h"


Keyer::Keyer(IKeyerCW *soundCW) {
  qDebug() << "Keyer constructor called";
  addKeyerCW(soundCW);
}

void Keyer::initKeyer(int wpm, Mode mode) {
  qDebug() << "Keyer initKeyer called";
  m_ditTime   = IKeyerCW::calculateDuration(DIT, wpm);
  m_dahTime   = IKeyerCW::calculateDuration(DAH, wpm);
  m_spaceTime = IKeyerCW::calculateDuration(INTER_ELEMENT_SPACE, wpm);
  m_mode      = mode;
}

void Keyer::onDit(bool pressed, KeyType keyType) {
  if (pressed) {
    m_ditPressed  = true;
    m_lastPressed = DIT;
    enqueue(DIT);
  } else {
    m_ditPressed = false;
  }
}

void Keyer::onDah(bool pressed, KeyType keyType) {
  if (pressed) {
    m_dahPressed  = true;
    m_lastPressed = DAH;
    enqueue(DAH);
  } else {
    m_dahPressed = false;
  }
}

void Keyer::enqueue(KeyerItem item) {
  if (m_queue.size() < 1 || !m_pending) {
    m_queue.push(item);
    m_lastQueued = item;
    if (!m_pending) {
      m_threadKeyer = std::thread(&Keyer::keyerCall, this);
      m_threadKeyer.detach();
    }
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
  const bool squeeze = m_ditPressed && m_dahPressed;
  m_pending = m_queue.size() > 0;

  if (m_pending) {
    m_lastSqueeze = squeeze;
    KeyerItem item = m_queue.front();
    m_queue.pop();
    playDitDah(item);

    // enqueued by pending.
    keyerCall();
  } else {
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
    } else if (m_mode == IAMBIC_B && m_lastSqueeze) {
      enqueue(reverse(m_lastQueued));
      m_lastSqueeze = false;
    }
  }
}

int Keyer::ditTime()   const { return m_ditTime; }
int Keyer::dahTime()   const { return m_dahTime; }
int Keyer::spaceTime() const { return m_spaceTime; }
