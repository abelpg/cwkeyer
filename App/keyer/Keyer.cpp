#include "Keyer.h"



Keyer::Keyer(IKeyerCW * soundCW){
  qDebug() << "Keyer constructor called";
  add_keyerCW(soundCW);
}

void Keyer::init_keyer(int wpm, Mode mode) {
  qDebug() << "Keyer init_keyer called";
  _dit_time = IKeyerCW::calculate_duration(DIT, wpm);
  _dah_time = IKeyerCW::calculate_duration(DAH, wpm);
  _space_time = IKeyerCW::calculate_duration(INTER_ELEMENT_SPACE, wpm);
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

void Keyer::add_keyerCW(IKeyerCW* keyerCW) {
  keyerCW_list.push_back(keyerCW);
}


void Keyer::play_dit_dah(KeyerItem item) {


  if (item == DIT) {
    for (IKeyerCW* keyerCW : keyerCW_list) {
      keyerCW->run_cw(DIT, _dit_time);
    }

    Utils::sleep_for(_dit_time + _space_time);
  } else if (item == DAH) {
    for (IKeyerCW* keyerCW : keyerCW_list) {
      keyerCW->run_cw(DAH, _dah_time);
    }
    Utils::sleep_for(_dah_time + _space_time);
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
