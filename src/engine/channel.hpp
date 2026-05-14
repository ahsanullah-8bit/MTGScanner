#pragma once

#include <qcamera.h>
#include <qmediacapturesession.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <qvideosink.h>
#include <tbb_patched.hpp>

#include <QPair>
#include <QThread>
#include <QCamera>
#include <QVideoSink>
#include <QAtomicInteger>
#include <QMediaCaptureSession>

#include <core/frame.hpp>
#include <engine/channelmetrics.h>
#include <engine/framespersecond.hpp>
#include <channeloptions.hpp>

namespace MTGS {

// This lives on the EngineWorker
struct ChannelRaw {
    struct {
        QThread* thread = nullptr;
        class CameraCapture* worker = nullptr;
    } capture;
    ChannelOptions options;

    QSharedPointer<tf::async_node<tf::continue_msg, FramePtr>> asyncSrc;
    QSharedPointer<tf::limiter_node<FramePtr>> preLimiter;
    QSharedPointer<tf::sequencer_node<FramePtr>> postSequencer;

    FramesPerSecond fps;
    FramesPerSecond skippedFps;
    FramesPerSecond captureFps;
    QAtomicInt totalSkippedFrames = 0;

    QAtomicInt status = 0;
    QAtomicInt visibleCards;
};

// This lives on the Engine (main thread).
class Channel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QCamera* camera READ camera WRITE setCamera NOTIFY cameraChanged FINAL)
    Q_PROPERTY(QMediaCaptureSession* captureSession READ captureSession WRITE setCaptureSession NOTIFY captureSessionChanged FINAL)
    Q_PROPERTY(QVideoSink* outVideoSink READ outVideoSink WRITE setOutVideoSink NOTIFY outVideoSinkChanged FINAL)
    Q_PROPERTY(ChannelOptions options READ options WRITE setOptions NOTIFY optionsChanged FINAL)
    Q_PROPERTY(ChannelMetrics* metrics READ metrics WRITE setMetrics NOTIFY metricsChanged FINAL)
public:
    explicit Channel(QObject *parent = nullptr) : QObject(parent) {}
    QCamera *camera();
    QMediaCaptureSession *captureSession();
    QVideoSink *outVideoSink();
    ChannelOptions &options();
    ChannelMetrics *metrics();

public slots:
    void setCamera(QCamera *camera);
    void setCaptureSession(QMediaCaptureSession *captureSession);
    void setOutVideoSink(QVideoSink *outVideoSink);
    void setOptions(const ChannelOptions &options);
    void setMetrics(ChannelMetrics *metrics);

signals:
    void cameraChanged();
    void captureSessionChanged();
    void outVideoSinkChanged();
    void optionsChanged();
    void metricsChanged();

private:
    QCamera *m_camera = nullptr;
    QMediaCaptureSession *m_captureSession = nullptr;
    QVideoSink* m_outVideoSink = nullptr;

    ChannelOptions m_options;
    ChannelMetrics *m_metrics = nullptr;
};

inline QCamera* Channel::camera() { return m_camera; }
inline QMediaCaptureSession* Channel::captureSession() { return m_captureSession; }
inline QVideoSink* Channel::outVideoSink() { return m_outVideoSink; }
inline ChannelOptions& Channel::options() { return m_options; }
inline ChannelMetrics* Channel::metrics() { return m_metrics; }

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

inline void Channel::setOptions(const ChannelOptions &options) {
    if (m_options == options) return;
    m_options = options;
    emit optionsChanged();
}

inline void Channel::setMetrics(ChannelMetrics *metrics) {
    if (m_metrics == metrics) return;
    m_metrics = metrics;
    emit metricsChanged();
}

} // namespace MTGS
