/*
 * Copyright (C) 2026 EA1FXG Abel
 * This file is part of CwKeyer and is licensed under the GNU General Public License v3.0 or later.
 * See LICENSE for details.
 */

#ifndef CWKEYERAPP_CWGENERATOR_H
#define CWKEYERAPP_CWGENERATOR_H

#include <QObject>
#include <QByteArray>
#include <cmath>
#include <limits>

class CwGenerator : public QObject {
    Q_OBJECT
public:
    explicit CwGenerator(int sampleRate, double frequency,
                         double amplitude, int attackSamples, int releaseSamples,
                         QObject *parent = nullptr);

    void startStream();
    void stopStream();
    bool isStopped() const;          // true cuando el release terminÃ³

    QByteArray generateChunk(int numSamples);

private:
    int    m_sampleRate;
    double m_twoPiF;
    double m_maxAmplitude;
    int    m_attackSamples;
    int    m_releaseSamples;

    qint64 m_sampleIndex = 0;
    bool   m_stopping    = false;
    bool   m_stopped     = false;
    qint64 m_stopSample  = 0;
};

#endif //CWKEYERAPP_CWGENERATOR_H

