#include "Sound.h"

Sound::Sound(QObject *parent) : QObject(parent) {
    connect(this, &Sound::playRequested,  this, &Sound::onPlayRequested,  Qt::QueuedConnection);
    connect(this, &Sound::startCwRequested, this, &Sound::onStartCwRequested, Qt::QueuedConnection);
    connect(this, &Sound::stopCwRequested,  this, &Sound::onStopCwRequested,  Qt::QueuedConnection);
}

Sound::~Sound() {
    stop();
    delete m_sink;
}

void Sound::initWithDevice(const QAudioDevice &device,
    double frequency, int sampleRate, double amplitude,
    double attackTime, double releaseTime) {

    log(L_DEBUG) << "Sound::initWithDevice() freq:" << frequency
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

    if (!m_device.isFormatSupported(format))
        throw std::runtime_error("format not supported");

    delete m_sink;
    m_sink = new QAudioSink(m_device, format, this);

    delete m_cwGenerator;
    m_cwGenerator = new CwGenerator(m_sampleRate, m_frequency, m_amplitude,
                                    m_attackSamples, m_releaseSamples, this);

    m_cacheSound.clear();
}

void Sound::init(double frequency, int sampleRate, double amplitude,
                 double attackTime, double releaseTime) {
    initWithDevice(QMediaDevices::defaultAudioOutput(),
                   frequency, sampleRate, amplitude, attackTime, releaseTime);
}

void Sound::setEnabled(bool enable) {
    m_enabled = enable;
    log(L_DEBUG) << "Sound::setEnabled()" << enable;
}

QByteArray Sound::generateBuffer(double durationSec) {
    const int totalSamples = static_cast<int>(m_sampleRate * durationSec);
    QByteArray buffer;
    buffer.resize(totalSamples * sizeof(qint16));
    qint16 *samples = reinterpret_cast<qint16 *>(buffer.data());

    for (int i = 0; i < totalSamples; ++i) {
        double t   = static_cast<double>(i) / m_sampleRate;
        double env = 1.0;
        if (i < m_attackSamples && m_attackSamples > 0) {
            env = static_cast<double>(i) / m_attackSamples;
        } else if (i >= (totalSamples - m_releaseSamples) && m_releaseSamples > 0)
            env = static_cast<double>(totalSamples - i) / m_releaseSamples;

        double sample = env * m_maxAmplitude * std::sin(m_twoPiF * t);
        samples[i] = static_cast<qint16>(qBound(-32768.0, sample, 32767.0));
    }
    return buffer;
}

void Sound::stop() {
    if (m_pushTimer) {
        m_pushTimer->stop();
        m_pushTimer->deleteLater();
        m_pushTimer  = nullptr;
        m_sinkDevice = nullptr;
    }

    if (m_sink) {
        m_sink->reset();   // vacía los buffers internos de Qt/hardware
    }
    if (m_sink && m_sink->state() != QAudio::StoppedState) {
        m_sink->stop();
    }
    if (m_activeBuffer) {
        m_activeBuffer->close();
        delete m_activeBuffer;
        m_activeBuffer = nullptr;
    }
    // No borramos m_cacheSound para reutilizar los buffers ya generados
}

void Sound::onPlayRequested(int duration) {
    if (!m_enabled) return;

    // stop() detiene el timer de push, para el sink y hace reset() de los buffers
    stop();

    if (!m_cacheSound.contains(duration)) {
        m_cacheSound[duration] = generateBuffer(duration / 1000.0);
    }

    log(L_DEBUG) << "Sound::onPlayRequested() INIT" << duration;
    m_activeBuffer = new QBuffer();
    m_activeBuffer->setData(m_cacheSound[duration]);
    m_activeBuffer->open(QIODevice::ReadOnly);
    m_sink->start(m_activeBuffer);
    log(L_DEBUG) << "Sound::onPlayRequested() END" << duration;
}

void Sound::onStartCwRequested() {
    if (!m_enabled || !m_cwGenerator) return;

    // Reiniciar el generador (ataque limpio desde 0)
    m_cwGenerator->startStream();

    // Si el timer ya está activo el sink sigue corriendo: solo hemos relanzado
    // el generador y no hay nada más que hacer. Evitamos detener/reiniciar el
    // sink entre elementos CW, que es la causa del corte de audio.
    if (m_pushTimer) return;

    // Primera vez (o tras un período largo de silencio): arrancar el sink.
    stop();   // limpia cualquier reproducción de tono puntual activa
    m_sinkDevice = m_sink->start();

    const int chunkMs    = 20;
    const int numSamples = m_sampleRate * chunkMs / 1000;

    // Ticks consecutivos de silencio (generador parado) antes de apagar el sink.
    // 2 s de margen: cubre cualquier latencia de hardware y pausa inter-elemento.
    // Si llega un nuevo onStartCwRequested antes, el timer ya está vivo y sigue.
    const int maxIdleTicks = 2000 / chunkMs;

    m_pushTimer = new QTimer(this);
    m_pushTimer->setInterval(chunkMs);

    connect(m_pushTimer, &QTimer::timeout, this,
            [this, numSamples, maxIdleTicks, idleTicks = 0]() mutable {
        if (!m_sinkDevice) return;

        // El generador produce tono con envolvente o silencio cuando está parado;
        // simplemente volcamos lo que genere sin lógica de drenado manual.
        QByteArray chunk = m_cwGenerator->generateChunk(numSamples);
        m_sinkDevice->write(chunk);

        if (m_cwGenerator->isStopped()) {
            // Silencio activo: contar hacia el apagado automático del sink
            if (++idleTicks >= maxIdleTicks) {
                m_pushTimer->stop();
                m_pushTimer->deleteLater();
                m_pushTimer  = nullptr;
                m_sinkDevice = nullptr;
                m_sink->stop();
            }
        } else {
            idleTicks = 0;  // tono activo: reiniciar contador
        }
    });
    m_pushTimer->start();
}

void Sound::onStopCwRequested() {
    if (!m_cwGenerator) return;
    // El generador aplica el release y luego emite silencio.
    // El timer sigue corriendo; el sink NO se detiene aquí para evitar
    // cortes cuando el siguiente elemento CW llegue poco después.
    m_cwGenerator->stopStream();
}

void Sound::runCW(KeyerItem item, int duration) {
    emit playRequested(duration);
}

void Sound::startRunCw() {
    emit startCwRequested();
}

void Sound::stopRunCw() {
    emit stopCwRequested();
}

void Sound::listDevices() {
    const QList<QAudioDevice> deviceInfos = QMediaDevices::audioOutputs();
    for (const QAudioDevice &deviceInfo : deviceInfos) {
        log(L_DEBUG) <<  "Device Name " << deviceInfo.id().toStdString();
    }
}