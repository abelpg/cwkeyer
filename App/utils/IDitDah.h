#ifndef CWKEYERAPP_IDITDAH_H
#define CWKEYERAPP_IDITDAH_H


class IDitDah{
  public:
    virtual ~IDitDah() = default;
    virtual void on_dah(bool pressed) = 0;
    virtual void on_dit(bool pressed) = 0;
};


#endif //CWKEYERAPP_IDITDAH_H
