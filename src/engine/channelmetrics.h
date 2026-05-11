#pragma once

#include <QObject>
#include <QColor>
#include <QTimer>
#include <QAtomicInteger>

#include <engine/framespersecond.hpp>

namespace MTGS {
    
class ChannelMetrics : public QObject {
    Q_OBJECT
    Q_PROPERTY(int status READ status NOTIFY fireMetricsUpdate FINAL)
    Q_PROPERTY(QColor statusColor READ statusColor NOTIFY fireMetricsUpdate FINAL)
    Q_PROPERTY(int fps READ fps NOTIFY fireMetricsUpdate FINAL)
    Q_PROPERTY(int captureFps READ captureFps NOTIFY fireMetricsUpdate FINAL)
    Q_PROPERTY(int skippedFps READ skippedFps NOTIFY fireMetricsUpdate FINAL)
    Q_PROPERTY(int visibleCards READ visibleCards NOTIFY fireMetricsUpdate FINAL)

public:
    explicit ChannelMetrics(QObject *parent = nullptr);
    int status() const;
    QColor statusColor() const;
    int fps() const;
    int captureFps() const;
    int skippedFps() const;
    int visibleCards() const;

public slots:
    void setStatus(QAtomicInt *);
    void setFps(FramesPerSecond *);
    void setCaptureFps(FramesPerSecond *);
    void setSkippedFps(FramesPerSecond *);
    void setVisibleCards(QAtomicInt *);

signals:
    void fireMetricsUpdate();

private:
    QAtomicInt *m_status = nullptr;
    FramesPerSecond *m_fps = nullptr;
    FramesPerSecond *m_captureFps = nullptr;
    FramesPerSecond *m_skippedFps = nullptr;
    QAtomicInt *m_visibleCards = nullptr;
};

}