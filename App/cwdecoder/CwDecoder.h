#ifndef CWKEYERAPP_CWDECODER_H
#define CWKEYERAPP_CWDECODER_H

#include "../utils/IKeyerCW.h"

#include <iostream>
#include <functional>

class CwDecoder : public IKeyerCW {

public:
   CwDecoder(std::function<void(const std::string &)> callbackTextDecoded);
   ~CwDecoder();

   bool started() const { return m_started; }
   void start(int farnsWorth);
   void stop();

private:
   int  m_interElementSpace = 0;
   int  m_letterSpace       = 0;
   int  m_wordSpace         = 0;
   bool m_started           = false;

   std::function<void(const std::string &)> m_callbackTextDecoded;

   void runCW(KeyerItem item, int duration) override;
};

#endif //CWKEYERAPP_CWDECODER_H
