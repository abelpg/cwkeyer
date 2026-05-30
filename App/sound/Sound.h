#ifndef CWKEYERAPP_SOUND_H
#define CWKEYERAPP_SOUND_H

#include <QObject>
#include <QTimer>
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
#include <stdexcept>
#include "CwGenerator.h"
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

  bool enabled() const { return m_enabled; }

  void runCW(KeyerItem item, int duration) override;
  void startRunCw() override;
  void stopRunCw() override;

public slots:
    void setEnabled(bool enable);

  signals:
      void playRequested(int durationMs);
  void startCwRequested();
  void stopCwRequested();

private slots:
  void onPlayRequested(int durationMs);
  void onStartCwRequested();
  void onStopCwRequested();

private:
  QByteArray generateBuffer(double durationSec);

  QAudioSink   *m_sink         = nullptr;
  QBuffer      *m_activeBuffer = nullptr;
  CwGenerator  *m_cwGenerator  = nullptr;
  QAudioDevice  m_device;

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

  std::map<int, QByteArray> m_cacheSound;
  // En la sección private:
  QIODevice    *m_sinkDevice   = nullptr;   // device de push del sink
  QTimer       *m_pushTimer    = nullptr;   // timer para escribir chunks
};


#endif //CWKEYERAPP_SOUND_H