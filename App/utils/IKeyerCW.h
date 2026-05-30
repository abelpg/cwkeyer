#ifndef CWKEYERAPP_IKEYERCW_H
#define CWKEYERAPP_IKEYERCW_H

// 1WPM dit = 1200 ms mark, 1200 ms space
static constexpr int PARIS_TIME_BASE = 1200;

enum KeyerItem {
  DIT = 0x1,
  DAH = 0x2,
  INTER_ELEMENT_SPACE = 0x3,
  LETTER_SPACE = 0x4,
  WORD_SPACE = 0x5
};

class IKeyerCW {
  public:
    virtual ~IKeyerCW() = default;
    virtual void runCW(KeyerItem item, int duration) = 0;
    virtual void startRunCw() = 0;
    virtual void stopRunCw() = 0;

  /**
    *
    So the word PARIS has been chosen to represent the standard word length for measuring the speed of sending CW.
    The word PARIS comprises a total of 50 units; one unit is the length of one dit.
    Those 50 units are made up of 22 mark units and 28 space units.
    Key Timing Formulas
    Dit Length () = 1200ms / WPM
    Dah Length () = 3x Dit Length
    Inter-element Space = 1 Dit Length
    Letter Space = 3 Dit Lengths
    Word Space = 7 Dit Lengths
    Example Speeds
    15 WPM: Dit = 80ms, Dah = 240ms
    20 WPM: Dit = 60ms, Dah = 180ms
    24 WPM: Dit = 50ms, Dah = 150ms
    30 WPM: Dit = 40ms, Dah = 120ms
    */
   static int calculateDuration(KeyerItem item, int wpm) {
     if (item == DIT) {
       return PARIS_TIME_BASE / wpm;
     } else if (item == DAH) {
       return PARIS_TIME_BASE / wpm * 3.0;
     } else if (item == INTER_ELEMENT_SPACE) {
       return PARIS_TIME_BASE / wpm;
     } else if (item == LETTER_SPACE) {
       return PARIS_TIME_BASE / wpm * 3.0;
     }
     return PARIS_TIME_BASE / wpm * 7.0;
   }
};


#endif //CWKEYERAPP_IKEYERCW_H
