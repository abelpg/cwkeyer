#ifndef CWKEYERAPP_SERIALPORTS_H
#define CWKEYERAPP_SERIALPORTS_H

#include <vector>
#include <string>
#include <iostream>
#ifdef _WIN32
#  include <windows.h>
#endif
#include "../utils/Logger.h"

class SerialPorts {
public:
  static std::vector<std::string> listPorts();
};

#endif //CWKEYERAPP_SERIALPORTS_H
