
#ifndef CWKEYERAPP_KEYER_H
#define CWKEYERAPP_KEYER_H

#include <QDebug>
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
    Keyer();


  private:
    bool dit_pressed, dah_pressed = false;


};


#endif //CWKEYERAPP_KEYER_H
