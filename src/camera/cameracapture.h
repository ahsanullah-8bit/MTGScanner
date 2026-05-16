#pragma once

#include <cstddef>
#include <tbb_patched.hpp>

#include <QObject>
#include <QCamera>
#include <QVideoFrame>
#include <QVideoSink>
#include <QCameraDevice>
#include <QMediaCaptureSession>
#include <QSharedPointer>

#include <core/frame.hpp>
#include <framespersecond.hpp>
#include <channelmetrics.h>

namespace MTGS {

class CameraCapture : public QObject {
    Q_OBJECT
public:
    CameraCapture(const QString &channelId,
                const QString &cameraId,
                FramesPerSecond &fps,
                QObject*parent = nullptr);

public slots:
    void setGateway(tbb::flow::async_node<tbb::flow::continue_msg, FramePtr>::gateway_type *gateway);
    void onVideoFrameChanged(const QVideoFrame &frame);
    void onErrorOccurred(const QString &channelName, QCamera::Error error, const QString &errorString);
    void onActiveChanged(const QString &channelName, bool isActive);

private:
    QString m_channelId;
    QString m_cameraId;
    size_t m_frameSequenceCount = 0;
    FramesPerSecond &m_fps;
    tbb::flow::async_node<tbb::flow::continue_msg, FramePtr>::gateway_type *m_gateway = nullptr;
};

} // namespace MTGS