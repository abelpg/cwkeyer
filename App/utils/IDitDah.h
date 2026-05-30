#ifndef CWKEYERAPP_IDITDAH_H
#define CWKEYERAPP_IDITDAH_H

enum KeyType {
  AUTOMATIC=0x1,
  MANUAL=0x2
};

class IDitDah {
  public:
    virtual ~IDitDah() = default;

    /**Dit dah from automatic keys**/
    virtual void onDah(bool pressed, KeyType keyType) = 0;
    virtual void onDit(bool pressed, KeyType keyType) = 0;

};


#endif //CWKEYERAPP_IDITDAH_H
