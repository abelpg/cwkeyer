//
// Created by Capitang7 on 30/05/2026.
//

#ifndef CWKEYERAPP_N1MMPROXY_H
#define CWKEYERAPP_N1MMPROXY_H

#include "SerialComm.h"
#include "../utils/IDitDah.h"

class N1MMProxy : public SerialComm {

  public:
  N1MMProxy(IDitDah *ditDah);

  private:
  IDitDah *m_ditDah;

};


#endif //CWKEYERAPP_N1MMPROXY_H
