#include "Sound.h"
#include <stdexcept>

Sound::Sound(QObject *parent) : QObject(parent) {
    qDebug() << "Sound constructor called";
}

Sound::~Sound() {
    qDebug() << "Sound destructor called";
    stop();
    delete sink;
}

void Sound::init(double frequency, int sampleRate, double amplitude,
                 double attackTime, double releaseTime)
{
    device = QMediaDevices::defaultAudioOutput();

    this->frequency    = frequency;
    this->sampleRate   = sampleRate;
    this->amplitude    = amplitude;
    this->attackTime   = attackTime;
    this->releaseTime  = releaseTime;

    attackSamples  = static_cast<int>(sampleRate * attackTime);
    releaseSamples = static_cast<int>(sampleRate * releaseTime);
    maxAmplitude   = amplitude * std::numeric_limits<qint16>::max();
    twoPiF         = 2.0 * M_PI * frequency;

    QAudioFormat format;
    format.setSampleRate(sampleRate);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    if (!device.isFormatSupported(format)) {
        throw std::runtime_error("format not supported");
    }

    // Crear sink único reutilizable
    delete sink;
    sink = new QAudioSink(device, format, this);

}

QByteArray Sound::generate_buffer(double durationSec) {
    const int totalSamples = static_cast<int>(sampleRate * durationSec);
    QByteArray buffer;
    buffer.resize(totalSamples * sizeof(qint16));
    qint16 *samples = reinterpret_cast<qint16 *>(buffer.data());

    for (int i = 0; i < totalSamples; ++i) {
        double t = static_cast<double>(i) / sampleRate;

        double env = 1.0;
        if (i < attackSamples && attackSamples > 0)
            env = static_cast<double>(i) / attackSamples;
        else if (i >= (totalSamples - releaseSamples) && releaseSamples > 0)
            env = static_cast<double>(totalSamples - i) / releaseSamples;

        double sample = env * maxAmplitude * std::sin(twoPiF * t);
        samples[i] = static_cast<qint16>(qBound(-32768.0, sample, 32767.0));
    }
    return buffer;
}

void Sound::stop() {
    if (sink && sink->state() != QAudio::StoppedState)
        sink->stop();
    if (activeBuffer) {
        activeBuffer->close();
        delete activeBuffer;
        activeBuffer = nullptr;
    }
}

void Sound::play_tone(int duration) {
    stop();  // detener reproducción anterior si hay

    // Pre-generar los 3 buffers
    //cache[ditMs]   = generate_buffer(duration   / 1000.0);
    //cache[dahMs]   = generate_buffer(dahMs   / 1000.0);

    activeBuffer = new QBuffer(this);
    activeBuffer->setData(generate_buffer(duration   / 1000.0));   // O(1) — solo referencia al cache
    activeBuffer->open(QIODevice::ReadOnly);

    sink->start(activeBuffer);
}

void Sound::run_cw(int duration) {
  play_tone(duration);
}

void Sound::list_devices() {
    const QList<QAudioDevice> deviceInfos = QMediaDevices::audioOutputs();
    for (const QAudioDevice &deviceInfo : deviceInfos)
        qDebug() << "Device name: " << deviceInfo.id();
}