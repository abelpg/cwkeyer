
#ifndef CWKEYERAPP_UTILS_H
#define CWKEYERAPP_UTILS_H

#include <unistd.h>
#include <set>
class Utils {

  public:
    static void sleep_for(int milliseconds);


  private:
    Utils();
};

class IDitDah{
  public:
  virtual void on_dah(bool pressed);
  virtual void on_dit(bool pressed);
};


#endif //CWKEYERAPP_UTILS_H
