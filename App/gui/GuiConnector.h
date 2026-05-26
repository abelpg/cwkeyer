
#ifndef CWKEYERAPP_GUICONNECTOR_H
#define CWKEYERAPP_GUICONNECTOR_H

#include <QObject>

#include "../sound/Sound.h"
#include "../usb/HidDevice.h"



class GuiConnector : public QObject{
  Q_OBJECT
  public:
    GuiConnector();
    ~GuiConnector();
    Q_INVOKABLE void detect_device();

  private:
    Sound* sound;
    HidDevice* device;

};


#endif //CWKEYERAPP_GUICONNECTOR_H
