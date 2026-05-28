#include "Keyer.h"

// 1WPM dit = 1200 ms mark, 1200 ms space
const int Keyer::TIME_BASE = 1200;
/**
  *
    So the word PARIS has been chosen to represent the standard word length for measuring the speed of sending CW.
    The word PARIS comprises a total of 50 units; one unit is the length of one dit. Those 50 units are made up of 22 mark units and 28 space units.
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
Keyer::Keyer(IKeyerCW * soundCW){
  qDebug() << "Keyer constructor called";
  this->soundCW = soundCW;
}

void Keyer::init_keyer(int wpm, Mode mode) {
  qDebug() << "Keyer init_keyer called";
  dit_time = TIME_BASE / wpm;
  dah_time = dit_time * 3.0;
  space_time = dit_time;
  this->mode = mode;
}


void Keyer::on_dit(bool pressed) {

  if (pressed) {
    dit_pressed = true;
    last_pressed = DIT;
    enqueue(DIT);
  } else {
    dit_pressed = false;
  }

}
void Keyer::on_dah(bool pressed) {
  if (pressed) {
    dah_pressed = true;
    last_pressed = DAH;
    enqueue(DAH);
  } else {
    dah_pressed = false;
  }
}

void Keyer::enqueue(KeyerItem item) {
  if (queue.size() < 1 || ! pending) {
    queue.push(item);
    last_queued = item;
    if (!pending) {
      thread_keyer =  std::thread(&Keyer::keyer_call, this);
      thread_keyer.detach();

    }
  }
}

void Keyer::play_dit_dah(KeyerItem item) {


  if (item == DIT) {
    soundCW->run_cw(dit_time);
    Utils::sleep_for(dit_time + space_time);
  } else if (item == DAH) {
    soundCW->run_cw(dah_time);
    Utils::sleep_for(dah_time + space_time);
  }



}

void Keyer::keyer_call() {
  const bool squeeze = dit_pressed && dah_pressed;
  pending = queue.size() > 0;

  if (pending) {
    last_squeeze = squeeze;
    KeyerItem item = queue.front();
    queue.pop();
    play_dit_dah(item);

    // enqueued by pending.
    keyer_call();
  } else {
    // Process mode
    if (squeeze) {
      if (mode == ULTIMATIC) {
        enqueue(last_pressed);
      } else if (mode == IAMBIC_B || mode == IAMBIC_A) {
        enqueue(reverse(last_queued));
      }
    } else if (dit_pressed) {
      enqueue(DIT);
    } else if (dah_pressed) {
      enqueue(DAH);
    } else if (mode == IAMBIC_B && last_squeeze) {
      enqueue(reverse(last_queued));
      last_squeeze = false;
    }
  }
}
