
#ifndef CWKEYERAPP_GUICONNECTOR_H
#define CWKEYERAPP_GUICONNECTOR_H

#include <QObject>
#include <QVariant>
#include <QObject>
#include <iostream>
#include <QVariant>
#include "../sound/Sound.h"
#include "../usb/UsbDevice.h"
#include "../keyer/Keyer.h"



class GuiConnector : public QObject{
  Q_OBJECT

  Q_PROPERTY(double amplitude  READ amplitude  WRITE setAmplitude  NOTIFY amplitudeChanged)
  Q_PROPERTY(double frequency  READ frequency  WRITE setFrequency  NOTIFY frequencyChanged)
  Q_PROPERTY(int    wpm        READ wpm        WRITE setWpm        NOTIFY wpmChanged)

  public:
    explicit  GuiConnector(QObject *parent = 0);

    // Getters
    double amplitude() const { return m_amplitude; }
    double frequency() const { return m_frequency; }
    int    wpm()       const { return m_wpm; }

    Q_INVOKABLE void init_device();
    Q_INVOKABLE void detect_device();
    Q_INVOKABLE void connect_device();
    Q_INVOKABLE void disconnect_device();
    Q_INVOKABLE void quit();

  public slots:
    void setAmplitude(double value);
    void setFrequency(double value);
    void setWpm(int value);

  signals:
    void device_updated(QVariant varData);
    void amplitudeChanged(double amplitude);
    void frequencyChanged(double frequency);
    void wpmChanged(int wpm);

  private:
    Sound* sound;
    UsbDevice* device;
    Keyer* keyer;

    double m_amplitude = 0.5;
    double m_frequency = 600.0;
    int    m_wpm       = 25;

    void reinit_sound();
    void reinit_keyer();
    void send_device_updated(Device * device);


};


#endif //CWKEYERAPP_GUICONNECTOR_H
