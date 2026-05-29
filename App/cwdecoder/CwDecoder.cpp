
#include "CwDecoder.h"

CwDecoder::CwDecoder(std::function<void(const std::string&)> callbackTextDecoded) {
  m_callbackTextDecoded = callbackTextDecoded;
}
CwDecoder::~CwDecoder() {
  // Nothing
}

void CwDecoder::setEnabled(bool enabled) {
  m_enabled = enabled;
  std::cout << "CwDecoder::setEnabled()";
  if (enabled) {
    for (int i = 0; i < 100; i++) {
      m_callbackTextDecoded("Decoded text " + std::to_string(i));
    }
  }
}

void CwDecoder::run_cw(int duration) {
  if (!m_enabled) {
    return;
  }

  // Aquí iría la lógica para decodificar el CW recibido.
  // Por ahora, solo imprimimos un mensaje de depuración.
  std::cout << "CwDecoder::run_cw() called with duration=" << duration << "ms";
}


