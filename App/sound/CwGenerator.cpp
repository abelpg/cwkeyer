#include "CwGenerator.h"

CwGenerator::CwGenerator(int sampleRate, double frequency,
                         double amplitude, int attackSamples, int releaseSamples,
                         QObject *parent)
    : QObject(parent),
      m_sampleRate(sampleRate),
      m_twoPiF(2.0 * M_PI * frequency),
      m_maxAmplitude(amplitude * std::numeric_limits<qint16>::max()),
      m_attackSamples(attackSamples),
      m_releaseSamples(releaseSamples)
{}

void CwGenerator::startStream() {
    m_sampleIndex = 0;
    m_stopping    = false;
    m_stopped     = false;
    m_stopSample  = 0;
}

void CwGenerator::stopStream() {
    if (!m_stopping) {
        m_stopping   = true;
        m_stopSample = m_sampleIndex;
    }
}

bool CwGenerator::isStopped() const { return m_stopped; }

QByteArray CwGenerator::generateChunk(int numSamples) {
    QByteArray buffer(numSamples * sizeof(qint16), 0);
    qint16 *out = reinterpret_cast<qint16 *>(buffer.data());

    for (int i = 0; i < numSamples; ++i) {
        double env = 1.0;

        if (m_sampleIndex < m_attackSamples && m_attackSamples > 0)
            env = static_cast<double>(m_sampleIndex) / m_attackSamples;

        if (m_stopping) {
            qint64 elapsed = m_sampleIndex - m_stopSample;
            if (elapsed >= m_releaseSamples) {
                out[i] = 0;
                m_stopped = true;
                ++m_sampleIndex;
                continue;
            }
            double rel = 1.0 - static_cast<double>(elapsed) / m_releaseSamples;
            env = qMin(env, rel);
        }

        double t      = static_cast<double>(m_sampleIndex) / m_sampleRate;
        double sample = env * m_maxAmplitude * std::sin(m_twoPiF * t);
        out[i] = static_cast<qint16>(qBound(-32768.0, sample, 32767.0));
        ++m_sampleIndex;
    }
    return buffer;
}