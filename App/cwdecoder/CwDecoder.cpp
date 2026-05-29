#include "CwDecoder.h"

CwDecoder::CwDecoder(std::function<void(const std::string &)> callbackTextDecoded) {
  m_callbackTextDecoded = callbackTextDecoded;
}

CwDecoder::~CwDecoder() {
  // Nothing
}

void CwDecoder::start(int farnsWorth) {
  m_started            = true;
  m_interElementSpace  = IKeyerCW::calculateDuration(KeyerItem::INTER_ELEMENT_SPACE, farnsWorth);
  m_letterSpace        = IKeyerCW::calculateDuration(KeyerItem::LETTER_SPACE,        farnsWorth);
  m_wordSpace          = IKeyerCW::calculateDuration(KeyerItem::WORD_SPACE,          farnsWorth);
}

void CwDecoder::stop() {
  m_started = false;
}

void CwDecoder::runCW(KeyerItem item, int duration) {
  if (!m_started) {
    return;
  }

  // Aquí iría la lógica para decodificar el CW recibido.
  // Por ahora, solo imprimimos un mensaje de depuración.
  std::cout << "CwDecoder::runCW() called with duration=" << duration << "ms";
}
