
#include "Utils.h"

void Utils::sleepFor(int milliseconds) {
  std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}
