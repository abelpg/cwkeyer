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
