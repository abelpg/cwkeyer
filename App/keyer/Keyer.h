
#ifndef CWKEYERAPP_KEYER_H
#define CWKEYERAPP_KEYER_H

#include <QDebug>
#include <queue>
#include <thread>
#include <iostream>
#include "../utils/Utils.h"
#include "../utils/IDitDah.h"
#include "../utils/IKeyerCW.h"


enum Mode {
  ULTIMATIC = 0x1,
  IAMBIC_A = 0x2,
  IAMBIC_B = 0x3
};

class Keyer : public IDitDah{

  public:
    Keyer(IKeyerCW * soundCW);
    void init_keyer(int wpm, Mode mode);
    void on_dit(bool pressed) override;
    void on_dah(bool pressed) override;

    int dit_time();
    int dah_time();
    int space_time();

    void add_keyerCW(IKeyerCW* keyerCW);

  private:

    int _dit_time, _dah_time, _space_time = 0;

    bool dit_pressed, dah_pressed, pending, last_squeeze = false;
    KeyerItem last_pressed, last_queued;
    Mode mode = IAMBIC_B;
    std::queue<KeyerItem> queue;
    std::thread thread_keyer;

    void enqueue(KeyerItem item);
    void keyer_call();
    void play_dit_dah(KeyerItem item);

    std::list<IKeyerCW*> keyerCW_list;

    static KeyerItem reverse(KeyerItem item) {
      if (item == DIT) {
        return DAH;
      } else {
        return DIT;
      }
    }

};


#endif //CWKEYERAPP_KEYER_H
