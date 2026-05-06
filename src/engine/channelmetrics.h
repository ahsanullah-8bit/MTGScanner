#pragma once

#include <QObject>
#include <QColor>
#include <QAtomicInteger>

namespace MTGS {
    
class ChannelMetrics : public QObject {
    Q_OBJECT
    Q_PROPERTY(int status READ status WRITE setStatus NOTIFY statusChanged FINAL)
    Q_PROPERTY(QColor statusColor READ statusColor NOTIFY statusColorChanged FINAL)
    Q_PROPERTY(int fps READ fps WRITE setFps NOTIFY fpsChanged FINAL)
    Q_PROPERTY(int captureFps READ captureFps WRITE setCaptureFps NOTIFY captureFpsChanged FINAL)
    Q_PROPERTY(int skippedFps READ skippedFps WRITE setSkippedFps NOTIFY skippedFpsChanged FINAL)
    Q_PROPERTY(int visibleCards READ visibleCards WRITE setVisibleCards NOTIFY visibleCardsChanged FINAL)

public:
    explicit ChannelMetrics(QObject *parent = nullptr);
    int status() const;
    QColor statusColor() const;
    int fps() const;
    int captureFps() const;
    int skippedFps() const;
    int visibleCards() const;

public slots:
    void setStatus(int);
    void setFps(int);
    void setCaptureFps(int);
    void setSkippedFps(int);
    void setVisibleCards(int);

signals:
    void statusChanged(int);
    void statusColorChanged();
    void fpsChanged(int);
    void captureFpsChanged(int);
    void skippedFpsChanged(int);
    void visibleCardsChanged(int);

private:
    QAtomicInt m_status = 0; // Which should be Unknown/Non-valid state.
    QAtomicInt m_fps;
    QAtomicInt m_captureFps;
    QAtomicInt m_skippedFps;
    QAtomicInt m_visibleCards;
};

}