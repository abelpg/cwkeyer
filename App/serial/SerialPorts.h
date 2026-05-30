
#ifndef CWKEYERAPP_SERIALPORTS_H
#define CWKEYERAPP_SERIALPORTS_H
#include <vector>
#include <iostream>
#include <windows.h>
#include "../utils/Logger.h"

class SerialPorts {

  public:
  static std::vector<std::string> listPorts();
};


#endif //CWKEYERAPP_SERIALPORTS_H
