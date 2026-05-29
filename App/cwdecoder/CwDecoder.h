#ifndef CWKEYERAPP_CWDECODER_H
#define CWKEYERAPP_CWDECODER_H

#include "../utils/IKeyerCW.h"
#include "MorseTable.h"

#include <iostream>
#include <functional>
#include <string>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>

class CwDecoder : public IKeyerCW {

public:
   CwDecoder(std::function<void(const std::string &)> callbackTextDecoded);
   ~CwDecoder();

   bool started() const { return m_started; }
   void start(int farnsWorth, int wpm);
   void stop();

private:
   int  m_interElementSpace = 0;
   int  m_letterSpace       = 0;
   int  m_wordSpace         = 0;
   bool m_started           = false;

   std::string m_currentSequence;

   std::chrono::steady_clock::time_point m_lastSignalTime;

   // Thread that flushes the last letter/word when silence exceeds the threshold
   std::thread        m_timeoutThread;
   std::atomic<bool>  m_running{false};
   std::mutex         m_mutex;           // protects m_currentSequence + m_lastSignalTime

   int                m_lastDuration = 0;

   std::function<void(const std::string &)> m_callbackTextDecoded;

   void runCW(KeyerItem item, int duration) override;
   void flushLetter();        // must be called with m_mutex held
   void timeoutLoop();        // runs in m_timeoutThread
};

#endif //CWKEYERAPP_CWDECODER_H