#ifndef CWKEYERAPP_SOUND_H
#define CWKEYERAPP_SOUND_H

#include <QObject>
#include <QDebug>
#include <QtMultimedia/QMediaDevices>
#include <QtMultimedia/QAudioDevice>
#include <QtMultimedia/QAudioSink>
#include <QtMultimedia/QAudioFormat>
#include <QByteArray>
#include <QBuffer>
#include <cmath>
#include <iostream>
#include <map>
#include <unordered_map>
#include <thread>
#include "../utils/IKeyerCW.h"

class Sound : public QObject, public IKeyerCW {
  Q_OBJECT

  public:

    explicit Sound(QObject *parent = nullptr);
    ~Sound();

    void init(double frequency, int sampleRate, double amplitude,
              double attackTime, double releaseTime);

    void initWithDevice(const QAudioDevice &device, double frequency, int sampleRate,
                        double amplitude, double attackTime, double releaseTime);

    void stop();
    void listDevices();

    // Getters
    bool enabled() const { return m_enabled; }

    // Override from IKeyerCW
    void runCW(KeyerItem item, int duration) override;  // duration en ms

  public slots:
    void setEnabled(bool enable);

  signals:
    void playRequested(int durationMs);      // emitida desde hilo del keyer

  private slots:
    void onPlayRequested(int durationMs);    // ejecutada en el hilo de Sound

  private:
    QByteArray generateBuffer(double durationSec);

    QAudioSink  *m_sink         = nullptr;
    QBuffer     *m_activeBuffer = nullptr;
    QAudioDevice m_device;

    bool   m_enabled        = true;
    int    m_sampleRate     = 44100;
    int    m_attackSamples  = 0;
    int    m_releaseSamples = 0;
    double m_maxAmplitude   = 0.0;
    double m_twoPiF         = 0.0;
    double m_frequency      = 0.0;
    double m_amplitude      = 0.0;
    double m_attackTime     = 0.0;
    double m_releaseTime    = 0.0;

    // Cache: buffers pre-generados para cada tipo
    std::map<int, QByteArray> m_cacheSound;

};

#endif //CWKEYERAPP_SOUND_H