/*
 * Copyright (C) 2026 EA1FXG Abel
 * This file is part of CwKeyer and is licensed under the GNU General Public License v3.0 or later.
 * See LICENSE for details.
 */
#ifndef CWKEYERAPP_UTILS_H
#define CWKEYERAPP_UTILS_H

#include <chrono>
#include <thread>
#include <set>

static uint64_t nowMs() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

class Utils {
  public:
    static void sleepFor(int milliseconds);

  private:
    Utils();
};


#endif //CWKEYERAPP_UTILS_H

