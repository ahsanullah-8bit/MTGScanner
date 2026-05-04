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

namespace MTGS {

class CameraCapture : public QObject {
    Q_OBJECT
public:
    CameraCapture(const QString &channelId, const QCameraDevice &device, tbb::flow::async_node<tbb::flow::continue_msg, FramePtr>::gateway_type &gateway, QObject*parent = nullptr);
    ~CameraCapture();
    QVideoSink *videoSink();
    QCameraDevice cameraDevice() const;

public slots:
    void init();
    void start();
    void stop();

signals:
    void errorOccurred(QCamera::Error error, const QString &errorString);

private:
    QString m_channelId;
    QCamera *m_camera;
    QCameraDevice m_cameraDevice;
    QMediaCaptureSession *m_captureSession;

    size_t m_frameSequenceCount = 0;
    tbb::flow::async_node<tbb::flow::continue_msg, FramePtr>::gateway_type &m_gateway;
};

} // namespace MTGS