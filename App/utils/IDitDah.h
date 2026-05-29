#ifndef CWKEYERAPP_IDITDAH_H
#define CWKEYERAPP_IDITDAH_H


class IDitDah {
  public:
    virtual ~IDitDah() = default;
    virtual void onDah(bool pressed) = 0;
    virtual void onDit(bool pressed) = 0;
};


#endif //CWKEYERAPP_IDITDAH_H
