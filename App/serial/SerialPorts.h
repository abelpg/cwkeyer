/*
 * Copyright (C) 2026 EA1FXG Abel
 * This file is part of CwKeyer and is licensed under the GNU General Public License v3.0 or later.
 * See LICENSE for details.
 */
#ifndef CWKEYERAPP_SERIALPORTS_H
#define CWKEYERAPP_SERIALPORTS_H

#include <vector>
#include <string>
#include "../utils/Logger.h"

class SerialPorts {
public:
  static std::vector<std::string> listPorts();
};

#endif //CWKEYERAPP_SERIALPORTS_H

