
#include "CwDecoder.h"

CwDecoder::CwDecoder(std::function<void(const std::string&)> callbackTextDecoded) {
  m_callbackTextDecoded = callbackTextDecoded;
}
CwDecoder::~CwDecoder() {
  // Nothing
}

void CwDecoder::start(int farnsWorth) {
  m_started = true;

  inter_element_space = IKeyerCW::calculate_duration(KeyerItem::INTER_ELEMENT_SPACE, farnsWorth);
  letter_space = IKeyerCW::calculate_duration(KeyerItem::LETTER_SPACE, farnsWorth);
  word_space = IKeyerCW::calculate_duration(KeyerItem::WORD_SPACE, farnsWorth);

}

void CwDecoder::stop() {
  m_started = false;
}

void CwDecoder::run_cw(KeyerItem item, int duration) {
  if (!m_started) {
    return;
  }

  // Aquí iría la lógica para decodificar el CW recibido.
  // Por ahora, solo imprimimos un mensaje de depuración.
  std::cout << "CwDecoder::run_cw() called with duration=" << duration << "ms";
}


