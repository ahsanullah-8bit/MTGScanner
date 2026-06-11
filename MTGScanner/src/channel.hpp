#pragma once

#include <QPair>
#include <QObject>
#include <QCamera>
#include <QVideoSink>
#include <QMediaPlayer>
#include <QMediaCaptureSession>
#include <QtQmlIntegration/qqmlintegration.h>

#include "channeloptions.hpp"
#include "channelmetrics.h"

namespace MTGS {

class AbstractChannel : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Channel objects are managed by the Engine and this is an abstract class")
protected:
    Q_PROPERTY(QString channelType READ channelType CONSTANT)
    Q_PROPERTY(ChannelOptions options READ constOptions WRITE setOptions NOTIFY optionsChanged FINAL)
    Q_PROPERTY(ChannelMetrics* metrics READ metrics WRITE setMetrics NOTIFY metricsChanged FINAL)
    Q_PROPERTY(QVideoSink* outVideoSink READ outVideoSink WRITE setOutVideoSink NOTIFY outVideoSinkChanged FINAL)
public:
    explicit AbstractChannel(const QString &channelType, QObject *parent = nullptr) : QObject(parent), m_channelType(channelType) {}
    virtual ~AbstractChannel() {}
    QString channelType() const;
    ChannelOptions &options();
    const ChannelOptions &constOptions() const;
    ChannelMetrics *metrics() const;
    QVideoSink *outVideoSink() const;
    virtual void start() = 0;
    virtual void stop() = 0;

public slots:
    void setOptions(const ChannelOptions &options);
    void setMetrics(ChannelMetrics *metrics);
    void setOutVideoSink(QVideoSink *outVideoSink);

signals:
    void optionsChanged();
    void metricsChanged();
    void outVideoSinkChanged();

private:
    QString m_channelType;
    ChannelOptions m_options;
    ChannelMetrics *m_metrics = nullptr;
    QVideoSink *m_outVideoSink = nullptr;
};

inline QString AbstractChannel::channelType() const { return m_channelType; }
inline ChannelOptions &AbstractChannel::options() { return m_options; }
inline const ChannelOptions &AbstractChannel::constOptions() const { return m_options; }
inline ChannelMetrics *AbstractChannel::metrics() const { return m_metrics; }
inline QVideoSink *AbstractChannel::outVideoSink() const { return m_outVideoSink; }

inline void AbstractChannel::setOptions(const ChannelOptions &options) {
    if (m_options == options) return;
    m_options = options;
    emit optionsChanged();
}

inline void AbstractChannel::setMetrics(ChannelMetrics *metrics) {
    if (m_metrics == metrics) return;
    m_metrics = metrics;
    emit metricsChanged();
}

inline void AbstractChannel::setOutVideoSink(QVideoSink *outVideoSink) {
    if (m_outVideoSink == outVideoSink) return;
    m_outVideoSink = outVideoSink;
    emit outVideoSinkChanged();
}

// This lives on the Engine (main thread).
class Channel : public AbstractChannel {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Channel objects are managed by the Engine")
    Q_PROPERTY(QCamera* camera READ camera WRITE setCamera NOTIFY cameraChanged FINAL)
    Q_PROPERTY(QMediaCaptureSession* captureSession READ captureSession WRITE setCaptureSession NOTIFY captureSessionChanged FINAL)
public:
    explicit Channel(QObject *parent = nullptr) : AbstractChannel("Channel", parent) {}
    QCamera *camera() const;
    QMediaCaptureSession *captureSession() const;
    void start() override;
    void stop() override;

public slots:
    void setCamera(QCamera *camera);
    void setCaptureSession(QMediaCaptureSession *captureSession);

signals:
    void cameraChanged();
    void captureSessionChanged();

private:
    QCamera *m_camera = nullptr;
    QMediaCaptureSession *m_captureSession = nullptr;
};

inline QCamera* Channel::camera() const { return m_camera; }
inline QMediaCaptureSession* Channel::captureSession() const { return m_captureSession; }
inline void Channel::start() { m_camera->start(); }
inline void Channel::stop() { m_camera->stop(); }

inline void Channel::setCamera(QCamera *camera) {
    if (m_camera == camera) return;
    m_camera = camera;
    emit cameraChanged();
}

inline void Channel::setCaptureSession(QMediaCaptureSession *captureSession) {
    if (m_captureSession == captureSession) return;
    m_captureSession = captureSession;
    emit captureSessionChanged();
}

class DemoChannel : public AbstractChannel {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("DemoChannel objects are managed by Engine")
public:
    explicit DemoChannel(QObject *parent = nullptr);
    QMediaPlayer *player() const;
    void start() override;
    void stop() override;

public slots:
    void setPlayer(QMediaPlayer *player);

private:
    QMediaPlayer *m_player = nullptr;
};

inline DemoChannel::DemoChannel(QObject *parent)
    : AbstractChannel("DemoChannel", parent)
{}

inline QMediaPlayer *DemoChannel::player() const { return m_player; }
inline void DemoChannel::start() { m_player->play(); }
inline void DemoChannel::stop() { m_player->pause(); }

inline void DemoChannel::setPlayer(QMediaPlayer *player)
{
    if (m_player == player) return;
    m_player = player;
}

} // namespace MTGS
