#include "Keyer.h"

// 1WPM dit = 1200 ms mark, 1200 ms space
const int Keyer::TIME_BASE = 1200;

Keyer::Keyer(int wpm) {
  qDebug() << "Keyer constructor called";

  dit_time = TIME_BASE / wpm;
  dah_time = dit_time * 3.0;
  space_time = dit_time;

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
  if (queue.size() == 1 || ! pending) {
    queue.push(item);
    last_queued = item;
    if (!pending) {
      thread_keyer =  std::thread(&Keyer::keyer_call, this);
    }
  }
}

void Keyer::play_dit_dah(KeyerItem item) {

  int time = space_time;
  if (item == DIT) {
    time+=dit_time;
    qDebug() << ".";
  } else if (item == DAH) {
    time+=dah_time;
    qDebug() << "-";
  }

  Utils::sleep_for(time);

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
        play_dit_dah(reverse(last_queued));
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
