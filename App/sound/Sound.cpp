#include "Sound.h"
#include <stdexcept>


Sound::Sound(QObject *parent) : QObject(parent) {
    // Conectar señal->slot en el mismo objeto: siempre QueuedConnection
    // para garantizar que onPlayRequested corre en el hilo de Sound
    connect(this, &Sound::playRequested,
            this, &Sound::onPlayRequested,
            Qt::QueuedConnection);
}

Sound::~Sound() {
    stop();
    delete m_sink;
}

void Sound::initWithDevice(const QAudioDevice &device,
    double frequency, int sampleRate, double amplitude,
    double attackTime, double releaseTime) {

    qDebug() << "Sound::initWithDevice() freq:" << frequency
             << " sampleRate:" << sampleRate << " amplitude:" << amplitude
             << " attackTime:" << attackTime << " releaseTime:" << releaseTime;

    m_device      = device;
    m_frequency   = frequency;
    m_sampleRate  = sampleRate;
    m_amplitude   = amplitude;
    m_attackTime  = attackTime;
    m_releaseTime = releaseTime;

    m_attackSamples  = static_cast<int>(m_sampleRate * m_attackTime);
    m_releaseSamples = static_cast<int>(m_sampleRate * m_releaseTime);
    m_maxAmplitude   = m_amplitude * std::numeric_limits<qint16>::max();
    m_twoPiF         = 2.0 * M_PI * m_frequency;

    QAudioFormat format;
    format.setSampleRate(m_sampleRate);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    if (!m_device.isFormatSupported(format)) {
        throw std::runtime_error("format not supported");
    }

    delete m_sink;
    m_sink = new QAudioSink(m_device, format, this);
    m_cacheSound.clear();
}

void Sound::init(double frequency, int sampleRate, double amplitude,
                 double attackTime, double releaseTime) {
    initWithDevice(QMediaDevices::defaultAudioOutput(),
                   frequency, sampleRate, amplitude, attackTime, releaseTime);
}

void Sound::setEnabled(bool enable) {
    m_enabled = enable;
    qDebug() << "Sound::setEnabled()" << enable;
}

QByteArray Sound::generateBuffer(double durationSec) {
    const int totalSamples = static_cast<int>(m_sampleRate * durationSec);
    QByteArray buffer;
    buffer.resize(totalSamples * sizeof(qint16));
    qint16 *samples = reinterpret_cast<qint16 *>(buffer.data());

    for (int i = 0; i < totalSamples; ++i) {
        double t = static_cast<double>(i) / m_sampleRate;

        double env = 1.0;
        if (i < m_attackSamples && m_attackSamples > 0) {
            env = static_cast<double>(i) / m_attackSamples;
        } else if (i >= (totalSamples - m_releaseSamples) && m_releaseSamples > 0) {
            env = static_cast<double>(totalSamples - i) / m_releaseSamples;
        }

        double sample = env * m_maxAmplitude * std::sin(m_twoPiF * t);
        samples[i] = static_cast<qint16>(qBound(-32768.0, sample, 32767.0));
    }
    return buffer;
}

void Sound::stop() {
    if (m_sink && m_sink->state() != QAudio::StoppedState) {
        m_sink->stop();
    }
    if (m_activeBuffer) {
        m_activeBuffer->close();
        delete m_activeBuffer;
        m_activeBuffer = nullptr;
    }
    m_cacheSound.clear();
}

// Ejecutado siempre en el hilo de Sound gracias a QueuedConnection
void Sound::onPlayRequested(int duration) {
    if (m_enabled) {
        stop();
        if (!m_cacheSound.contains(duration)) {
            m_cacheSound[duration] = generateBuffer(duration / 1000.0);
        }

        m_activeBuffer = new QBuffer();          // sin parent — gestionado por stop()
        m_activeBuffer->setData(m_cacheSound[duration]);
        m_activeBuffer->open(QIODevice::ReadOnly);

        m_sink->start(m_activeBuffer);
    }
}

void Sound::runCW(KeyerItem item, int duration) {
    emit playRequested(duration);
}

void Sound::listDevices() {
    const QList<QAudioDevice> deviceInfos = QMediaDevices::audioOutputs();
    for (const QAudioDevice &deviceInfo : deviceInfos)
        qDebug() << "Device name: " << deviceInfo.id();
}