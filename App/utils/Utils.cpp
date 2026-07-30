/*
 * Copyright (C) 2026 EA1FXG Abel
 * This file is part of CwKeyer and is licensed under the GNU General Public License v3.0 or later.
 * See LICENSE for details.
 */
#include "Utils.h"


#include <unistd.h>


/**
 * Sleep for the specified number of milliseconds, dont use std::this_thread::sleep_for(). Is broken.
 * @param milliseconds
 */
void Utils::sleepFor(int milliseconds) {
  usleep(milliseconds  * 1000);
}

