#ifndef CWKEYERAPP_IKEYERCW_H
#define CWKEYERAPP_IKEYERCW_H


class IKeyerCW {
  public:
    virtual ~IKeyerCW() = default;
    virtual void run_cw(int duration) = 0;
};


#endif //CWKEYERAPP_IKEYERCW_H
