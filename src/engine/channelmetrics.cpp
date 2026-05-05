#include "channelmetrics.h"
#include <qcolor.h>
#include <qobject.h>

namespace MTGS {

ChannelMetrics::ChannelMetrics(QObject *parent) : QObject(parent) {}

int ChannelMetrics::status() const
{
    return m_status.loadAcquire();
}

QColor ChannelMetrics::statusColor() const
{
    static QList<QColor> status_colors = 
    {
        "#95a5a6", // "Initializing"
        "#f1c40f", // "Starting"
        "#2ecc71", // "Running"
        "#7f8c8d", // "Stopping"
        "#e74c3c", // "Stopped"
        "#34495e"  // "Errored"
    };

    return status_colors.at(m_status.loadAcquire() % status_colors.size());
}

int ChannelMetrics::fps() const
{
    return m_fps.loadAcquire();
}

int ChannelMetrics::skippedFps() const
{
    return m_skippedFps.loadAcquire();
}

int ChannelMetrics::visibleCards() const
{
    return m_visibleCards.loadAcquire();
}

void ChannelMetrics::setStatus(int status)
{
    if (m_status.loadRelaxed() == status)
        return;

    m_status.storeRelease(status);
    emit statusChanged(status);
}

void ChannelMetrics::setFps(int fps)
{
    if (m_fps.loadRelaxed() == fps)
        return;

    m_fps.storeRelease(fps);
    emit fpsChanged(fps);
}

void ChannelMetrics::setSkippedFps(int skippedFps)
{
    if (m_skippedFps.loadRelaxed() == skippedFps)
        return;

    m_skippedFps.storeRelease(skippedFps);
    emit skippedFpsChanged(skippedFps);
}

void ChannelMetrics::setVisibleCards(int visibleCards)
{
    if (m_visibleCards.loadRelaxed() == visibleCards)
        return;

    m_visibleCards.storeRelease(visibleCards);
    emit visibleCardsChanged(visibleCards);
}

}