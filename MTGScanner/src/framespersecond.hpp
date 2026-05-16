#pragma once

#include <QDateTime>
#include <QList>
#include <QReadWriteLock>
#include <qdatetime.h>
#include <qminmax.h>
#include <qreadwritelock.h>
#include <qtypes.h>

class FramesPerSecond {
public:
    explicit FramesPerSecond(int maxEvents = 1000, int lastNSeconds = 10)
        : m_start(0.0)
        , m_maxEvents(maxEvents)
        , m_lastNSeconds(lastNSeconds) 
    {}

    void start() {
        QWriteLocker locker(&m_lock);
        m_start = QDateTime::currentDateTime().toSecsSinceEpoch();
        m_timestamps.clear();
    }

    void update() {
        QWriteLocker locker(&m_lock);
        const qreal now = QDateTime::currentDateTime().toSecsSinceEpoch();
        if (m_start == 0.0)
            m_start = now;

        m_timestamps.append(now);
        if (m_timestamps.length() > m_maxEvents + 100)
            m_timestamps = m_timestamps.mid(m_timestamps.length() - m_maxEvents);

        // Expire
        const qreal threshold = now - m_lastNSeconds;
        while (!m_timestamps.isEmpty() && m_timestamps.first() < threshold) {
            m_timestamps.removeFirst();
        }
    }

    qreal fps() {
        QReadLocker locker(&m_lock);
        if (m_start == 0.0)
            return 0.0;

        const qreal now = QDateTime::currentDateTime().toSecsSinceEpoch();
        const qreal threshold = now - m_lastNSeconds;

        int count = 0;
        for (const qreal &t : m_timestamps) {
            if (t >= threshold)
                ++count;
        }

        qreal seconds = qMin(now - m_start, static_cast<qreal>(m_lastNSeconds));
        if (seconds == 0.0)
            seconds = 1.0;  // avoid division-by-zero

        return count / seconds;
    }

    int eventCount() const {
        QReadLocker locker(&m_lock);
        if (m_start == 0.0)
            return 0.0;

        const qreal now = QDateTime::currentDateTime().toSecsSinceEpoch();
        const qreal threshold = now - m_lastNSeconds;
        
        int count = 0;
        for (const qreal &t : m_timestamps) {
            if (t >= threshold)
                ++count;
        }

        return count;
    }

private:
    mutable QReadWriteLock m_lock;
    qreal m_start;
    int m_maxEvents;
    int m_lastNSeconds;
    QList<qreal> m_timestamps;
};