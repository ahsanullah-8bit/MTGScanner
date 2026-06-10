#pragma once

#include <QPair>
#include <QObject>
#include <QCamera>
#include <QVideoSink>
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
    Q_PROPERTY(ChannelOptions options READ constOptions WRITE setOptions NOTIFY optionsChanged FINAL)
    Q_PROPERTY(ChannelMetrics* metrics READ metrics WRITE setMetrics NOTIFY metricsChanged FINAL)
public:
    explicit AbstractChannel(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~AbstractChannel() {}
    ChannelOptions &options();
    const ChannelOptions &constOptions() const;
    ChannelMetrics *metrics() const;

public slots:
    void setOptions(const ChannelOptions &options);
    void setMetrics(ChannelMetrics *metrics);

signals:
    void optionsChanged();
    void metricsChanged();

private:
    ChannelOptions m_options;
    ChannelMetrics *m_metrics = nullptr;
};

inline ChannelOptions &AbstractChannel::options() { return m_options; }
inline const ChannelOptions &AbstractChannel::constOptions() const { return m_options; }
inline ChannelMetrics *AbstractChannel::metrics() const { return m_metrics; }

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

// This lives on the Engine (main thread).
class Channel : public AbstractChannel {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Channel objects are managed by the Engine")
    Q_PROPERTY(QCamera* camera READ camera WRITE setCamera NOTIFY cameraChanged FINAL)
    Q_PROPERTY(QMediaCaptureSession* captureSession READ captureSession WRITE setCaptureSession NOTIFY captureSessionChanged FINAL)
    Q_PROPERTY(QVideoSink* outVideoSink READ outVideoSink WRITE setOutVideoSink NOTIFY outVideoSinkChanged FINAL)
public:
    explicit Channel(QObject *parent = nullptr) : AbstractChannel(parent) {}
    QCamera *camera() const;
    QMediaCaptureSession *captureSession() const;
    QVideoSink *outVideoSink() const;

public slots:
    void setCamera(QCamera *camera);
    void setCaptureSession(QMediaCaptureSession *captureSession);
    void setOutVideoSink(QVideoSink *outVideoSink);

signals:
    void cameraChanged();
    void captureSessionChanged();
    void outVideoSinkChanged();

private:
    QCamera *m_camera = nullptr;
    QMediaCaptureSession *m_captureSession = nullptr;
    QVideoSink* m_outVideoSink = nullptr;
};

inline QCamera* Channel::camera() const { return m_camera; }
inline QMediaCaptureSession* Channel::captureSession() const { return m_captureSession; }
inline QVideoSink* Channel::outVideoSink() const { return m_outVideoSink; }

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

inline void Channel::setOutVideoSink(QVideoSink *outVideoSink) {
    if (m_outVideoSink == outVideoSink) return;
    m_outVideoSink = outVideoSink;
    emit outVideoSinkChanged();
}

} // namespace MTGS
