
#ifndef CWKEYERAPP_SERIALPORTS_H
#define CWKEYERAPP_SERIALPORTS_H
#include <vector>
#include <iostream>
#include <windows.h>

class SerialPorts {

  public:
  static std::vector<std::string> listPorts();
};


#endif //CWKEYERAPP_SERIALPORTS_H
