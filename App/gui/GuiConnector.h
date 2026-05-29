#ifndef CWKEYERAPP_GUICONNECTOR_H
#define CWKEYERAPP_GUICONNECTOR_H

#include <QDebug>
#include <QApplication>
#include <QObject>
#include <iostream>
#include <QVariant>
#include <QtMultimedia/QMediaDevices>
#include <QtMultimedia/QAudioDevice>
#include "../sound/Sound.h"


#include "../usb/UsbDevice.h"
#include "../keyer/Keyer.h"
#include "../keyboard/Keyboard.h"
#include "../keyboard/KeyboardListener.h"
#include "../serial/SerialComm.h"
#include "../cwdecoder/CwDecoder.h"

static constexpr const int    DEFAULT_WPM         = 25;
static constexpr const int    DEFAULT_FARNSWORTH  = 25;
static constexpr const int    DEFAULT_SAMPLE_RATE = 44100;
static constexpr const int    DEFAULT_FREQUENCY   = 650;
static constexpr const double DEFAULT_AMPLITUDE   = 0.5;
static constexpr const double DEFAULT_ATTACK      = 0.01;
static constexpr const double DEFAULT_RELEASE     = 0.01;
static constexpr const Mode   DEFAULT_MODE        = IAMBIC_B;


class GuiConnector : public QObject{
  Q_OBJECT

  Q_PROPERTY(double      amplitude            READ amplitude            WRITE setAmplitude            NOTIFY amplitudeChanged)
  Q_PROPERTY(double      frequency            READ frequency            WRITE setFrequency            NOTIFY frequencyChanged)
  Q_PROPERTY(int         wpm                  READ wpm                  WRITE setWpm                  NOTIFY wpmChanged)
  Q_PROPERTY(int         farnsWorth           READ farnsWorth           WRITE setFarnsWorth           NOTIFY farnsWorthChanged)
  Q_PROPERTY(int         mode                 READ mode                 WRITE setMode                 NOTIFY modeChanged)
  Q_PROPERTY(int         enabledSound         READ enabledSound         WRITE setEnabledSound         NOTIFY soundEnabledChanged)
  Q_PROPERTY(int         enabledCommOut       READ enabledCommOut       WRITE setEnabledCommOut       NOTIFY enabledCommOutChanged)
  Q_PROPERTY(bool        enabledKeyboard      READ enabledKeyboard      WRITE setEnabledKeyboard      NOTIFY enabledKeyboardChanged)
  Q_PROPERTY(int         selectedAudioDevice  READ selectedAudioDevice  WRITE setSelectedAudioDevice  NOTIFY selectedAudioDeviceChanged)
  Q_PROPERTY(int         selectedCommPort     READ selectedCommPort     WRITE setSelectedCommPort     NOTIFY selectedCommPortChanged)
  Q_PROPERTY(bool        enabledCwDecoder     READ enabledCwDecoder     WRITE setEnabledCwDecoder     NOTIFY enabledCwDecoderChanged)
  Q_PROPERTY(QStringList audioDevices         READ audioDevices         NOTIFY audioDevicesChanged)
  Q_PROPERTY(QStringList commPorts            READ commPorts            NOTIFY commPortsChanged)

  public:
    explicit GuiConnector(QApplication *app, QObject *parent = nullptr);

    // Getters
    double      amplitude()           const { return m_amplitude; }
    double      frequency()           const { return m_frequency; }
    int         wpm()                 const { return m_wpm; }
    int         farnsWorth()          const { return m_farnsWorth; }
    QStringList audioDevices()        const { return m_audioDevices; }
    int         selectedAudioDevice() const { return m_selectedAudioDevice; }
    int         mode()                const { return m_mode; }
    bool        enabledSound()        const;
    bool        enabledCommOut()      const;
    bool        enabledKeyboard()     const;
    bool        enabledCwDecoder()    const;
    QStringList commPorts()           const { return m_commPorts; }
    int         selectedCommPort()    const { return m_selectedCommPort; }

    Q_INVOKABLE void initDevice();
    Q_INVOKABLE void detectDevice();
    Q_INVOKABLE void connectDevice();
    Q_INVOKABLE void disconnectDevice();
    Q_INVOKABLE void quit();

  public slots:
    void setAmplitude(double value);
    void setFrequency(double value);
    void setWpm(int value);
    void setFarnsWorth(int value);
    void setSelectedAudioDevice(int index);
    void setMode(int value);
    void setEnabledSound(bool enabled);
    void setEnabledCommOut(bool enabled);
    void setEnabledKeyboard(bool enabled);
    void setEnabledCwDecoder(bool enabled);
    void setSelectedCommPort(int index);

  signals:
    void textCwDecoderUpdated(QVariant varData);
    void deviceUpdated(QVariant varData);
    void amplitudeChanged(double amplitude);
    void frequencyChanged(double frequency);
    void wpmChanged(int wpm);
    void farnsWorthChanged(int farnsWorth);
    void audioDevicesChanged(QStringList devices);
    void selectedAudioDeviceChanged(int index);
    void modeChanged(int mode);
    void soundEnabledChanged(bool enabled);
    void commPortsChanged(QStringList ports);
    void selectedCommPortChanged(int index);
    void enabledCommOutChanged(bool enabled);
    void enabledKeyboardChanged(bool enabled);
    void enabledCwDecoderChanged(bool enabled);

  private:
    Sound            *m_sound;
    UsbDevice        *m_device;
    Keyer            *m_keyer;
    SerialComm       *m_serialComm;
    Keyboard         *m_keyboard;
    KeyboardListener *m_keyboardListener;
    QApplication     *m_app;
    CwDecoder        *m_cwDecoder;

    double m_amplitude  = DEFAULT_AMPLITUDE;
    double m_frequency  = DEFAULT_FREQUENCY;
    int    m_wpm        = DEFAULT_WPM;
    int    m_farnsWorth = DEFAULT_FARNSWORTH;

    QStringList          m_audioDevices;
    QList<QAudioDevice>  m_audioDeviceList;
    int                  m_selectedAudioDevice = 0;
    int                  m_mode               = static_cast<int>(Mode::IAMBIC_B);

    QStringList m_commPorts;
    int         m_selectedCommPort = -1;

    void resetSound();
    void resetKeyer();
    void resetCwDecoder();
    void sendDeviceUpdated(Device *device);
    void loadAudioDevices();
    void loadConfiguration();
    void loadCommPorts();
    void onDecodeTextCw(std::string text);
};


#endif //CWKEYERAPP_GUICONNECTOR_H
