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

  void stop();
  void list_devices();
  void run_cw(int duration) override;   // duration en ms

private:
  void play_tone(int durationMs);
  QByteArray generate_buffer(double durationSec);

  QAudioSink   *sink      = nullptr;
  QBuffer      *activeBuffer = nullptr;
  QAudioDevice  device;

  int    sampleRate    = 44100;
  int    attackSamples = 0;
  int    releaseSamples= 0;
  double maxAmplitude  = 0.0;
  double twoPiF        = 0.0;
  double frequency     = 0.0;
  double amplitude     = 0.0;
  double attackTime    = 0.0;
  double releaseTime   = 0.0;

  // Cache: buffers pre-generados para cada tipo
  //QByteArray cacheDit;   // índice = durationMs

};

#endif //CWKEYERAPP_SOUND_H