#include "Utils.h"


#include <unistd.h>


/**
 * Sleep for the specified number of milliseconds, dont use std::this_thread::sleep_for(). Is broken.
 * @param milliseconds
 */
void Utils::sleepFor(int milliseconds) {
  usleep(milliseconds  * 1000);
}
