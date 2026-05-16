#include "channelmetrics.h"

namespace MTGS {

ChannelMetrics::ChannelMetrics(QObject *parent)
    : QObject(parent)
{}

int ChannelMetrics::status() const
{
    if (!m_status)
        return 0;

    return m_status->loadAcquire();
}

QColor ChannelMetrics::statusColor() const
{
    static QList<QColor> status_colors = 
    {
        "red",       // "Uknown"
        "#95a5a6", // "Initializing"
        "#f1c40f", // "Starting"
        "#2ecc71", // "Running"
        "#7f8c8d", // "Stopping"
        "#e74c3c", // "Stopped"
        "#34495e"  // "Errored"
    };

    if (!m_status)
        return status_colors.at(0);

    return status_colors.at(m_status->loadAcquire() % status_colors.size());
}

int ChannelMetrics::fps() const
{
    if (!m_fps)
        return -1;

    return m_fps->fps();
}

int ChannelMetrics::captureFps() const
{
    if (!m_captureFps)
        return -1;

    return m_captureFps->fps();
}

int ChannelMetrics::skippedFps() const
{
    if (!m_skippedFps)
        return -1;

    return m_skippedFps->fps();
}

int ChannelMetrics::visibleCards() const
{
    if (!m_visibleCards)
        return -1;

    return m_visibleCards->loadAcquire();
}

void ChannelMetrics::setStatus(QAtomicInt *status)
{
    if (m_status == status)
        return;

    m_status = status;
}

void ChannelMetrics::setFps(FramesPerSecond *fps)
{
    if (m_fps == fps)
        return;

    m_fps = fps;
}

void ChannelMetrics::setCaptureFps(FramesPerSecond *captureFps)
{
    if (m_captureFps == captureFps)
        return;

    m_captureFps = captureFps;
}

void ChannelMetrics::setSkippedFps(FramesPerSecond *skippedFps)
{
    if (m_skippedFps == skippedFps)
        return;

    m_skippedFps = skippedFps;
}

void ChannelMetrics::setVisibleCards(QAtomicInt *visibleCards)
{
    if (m_visibleCards == visibleCards)
        return;

    m_visibleCards = visibleCards;
}

}