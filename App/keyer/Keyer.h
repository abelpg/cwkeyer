#ifndef CWKEYERAPP_KEYER_H
#define CWKEYERAPP_KEYER_H

#include <queue>
#include <thread>
#include <iostream>
#include <list>

#include "../utils/Utils.h"
#include "../utils/Logger.h"
#include "../utils/IDitDah.h"
#include "../utils/IKeyerCW.h"


enum Mode {
  ULTIMATIC = 0x1,
  IAMBIC_A  = 0x2,
  IAMBIC_B  = 0x3
};

class Keyer : public IDitDah {

  public:
    Keyer(IKeyerCW *soundCW);
    void initKeyer(int wpm, Mode mode);

    /**On dit automatic keys**/
    void onDit(bool pressed) override;
    void onDah(bool pressed) override;
    void onStraight(bool pressed) override;

    /****/

    int ditTime()   const;
    int dahTime()   const;
    int spaceTime() const;

    void addKeyerCW(IKeyerCW *keyerCW);

  private:

    int m_ditTime   = 0;
    int m_dahTime   = 0;
    int m_spaceTime = 0;

    bool      m_ditPressed  = false;
    bool      m_dahPressed  = false;
    bool      m_pending     = false;
    bool      m_lastSqueeze = false;
    KeyerItem m_lastPressed;
    KeyerItem m_lastQueued;
    Mode      m_mode = IAMBIC_B;

    std::queue<KeyerItem>  m_queue;
    std::thread            m_threadKeyer;
    std::list<IKeyerCW *>  m_keyerCWList;

    void enqueue(KeyerItem item);
    void keyerCall();
    void playDitDah(KeyerItem item);

    static KeyerItem reverse(KeyerItem item) {
      return (item == DIT) ? DAH : DIT;
    }

};


#endif //CWKEYERAPP_KEYER_H
