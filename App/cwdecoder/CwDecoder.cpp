#include "CwDecoder.h"

CwDecoder::CwDecoder(std::function<void(const std::string &)> callbackTextDecoded) {
  m_callbackTextDecoded = callbackTextDecoded;
}

CwDecoder::~CwDecoder() {
  stop();
}

void CwDecoder::start(int farnsWorth, int m_wpm) {
  stop(); // clean up any previous run

  m_interElementSpace = IKeyerCW::calculateDuration(KeyerItem::INTER_ELEMENT_SPACE,      m_wpm);
  m_letterSpace       = IKeyerCW::calculateDuration(KeyerItem::LETTER_SPACE,             m_wpm);
  m_wordSpace         = IKeyerCW::calculateDuration(KeyerItem::WORD_SPACE,          farnsWorth);

  m_currentSequence.clear();
  m_lastSignalTime = std::chrono::steady_clock::now();
  m_started        = true;
  m_running        = true;

  m_timeoutThread = std::thread(&CwDecoder::timeoutLoop, this);
}

void CwDecoder::stop() {
  m_running = false;
  m_started = false;
  if (m_timeoutThread.joinable()) {
    m_timeoutThread.join();
  }
  m_currentSequence.clear();
}

// ── Private helpers ───────────────────────────────────────────────────────────

// Must be called with m_mutex held
void CwDecoder::flushLetter() {
  if (m_currentSequence.empty()) return;

  char ch = MorseTable::decode(m_currentSequence);
  if (ch != '\0') {
    m_callbackTextDecoded(std::string(1, ch));
  }
  m_currentSequence.clear();
}

// Background thread: wakes up every ~20 ms and checks elapsed silence
void CwDecoder::timeoutLoop() {
  while (m_running) {
    std::this_thread::sleep_for(std::chrono::milliseconds(m_interElementSpace/2));

    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_currentSequence.empty()) continue;

    auto now       = std::chrono::steady_clock::now();
    int  silenceMs = static_cast<int>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            now - m_lastSignalTime).count());

    silenceMs = std::max(silenceMs - m_lastDuration - m_interElementSpace, m_interElementSpace);

    if (silenceMs >= m_wordSpace) {
      // Silence >= word space → flush letter + emit word space
      std::cout << "Timeout: silence " << silenceMs << " ms >= word space " << m_wordSpace << " ms\n";
      flushLetter();
      m_callbackTextDecoded(" ");
      m_lastDuration = 0;
    }
  }
}

// ── IKeyerCW override ─────────────────────────────────────────────────────────

void CwDecoder::runCW(KeyerItem item, int duration) {
  if (!m_started) return;

  std::lock_guard<std::mutex> lock(m_mutex);

  auto now       = std::chrono::steady_clock::now();


  int silenceMs = static_cast<int>(
      std::chrono::duration_cast<std::chrono::milliseconds>(
          now - m_lastSignalTime).count()) ;

  silenceMs = std::max(silenceMs - m_lastDuration - m_interElementSpace, m_interElementSpace);

  // Update timestamp AFTER processing
  m_lastSignalTime = std::chrono::steady_clock::now();
  m_lastDuration = duration;


  // Evaluate silence BEFORE this new signal
  if (silenceMs >= m_wordSpace) {
    flushLetter();
    m_callbackTextDecoded(" ");
  } else if (silenceMs > m_interElementSpace) {
    flushLetter();
  }

  // Accumulate the incoming element
  m_currentSequence += MorseTable::toSymbol(item);

}