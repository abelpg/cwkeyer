
#ifndef CWKEYERAPP_KEYER_H
#define CWKEYERAPP_KEYER_H

#include <QDebug>
#include <queue>
#include <thread>
#include "../utils/Utils.h"
enum KeyerItem {
  DIT = 0x1,
  DAH = 0x2
};

enum Mode {
  ULTIMATIC = 0x1,
  IAMBIC_A = 0x2,
  IAMBIC_B = 0x3
};



class Keyer : public IDitDah{

  public:
    Keyer(int wpm);
    void on_dit(bool pressed) override;
    void on_dah(bool pressed) override;


  private:

    const static int TIME_BASE;

    int dit_time, dah_time, space_time = 0;

    bool dit_pressed, dah_pressed, pending, last_squeeze = false;
    KeyerItem last_pressed, last_queued;
    Mode mode = IAMBIC_B;
    std::queue<KeyerItem> queue;
    std::thread thread_keyer;
    void enqueue(KeyerItem item);
    void keyer_call();
    void play_dit_dah(KeyerItem item);

    static KeyerItem reverse(KeyerItem item) {
      if (item == DIT) {
        return DAH;
      } else {
        return DIT;
      }
    }

};


#endif //CWKEYERAPP_KEYER_H
