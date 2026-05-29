
#ifndef CWKEYERAPP_CWDECODER_H
#define CWKEYERAPP_CWDECODER_H

#include "../utils/IKeyerCW.h"

#include <iostream>
#include <functional>
class CwDecoder : public IKeyerCW {

public:
   CwDecoder(std::function<void(const std::string&)> callbackTextDecoded);
   ~CwDecoder();
   bool enabled() const { return m_enabled; };
   void setEnabled(bool enabled);

private:
   bool m_enabled = false;
   std::function<void(const std::string&)> m_callbackTextDecoded;
   void run_cw(int duration) override;

};

#endif //CWKEYERAPP_CWDECODER_H
