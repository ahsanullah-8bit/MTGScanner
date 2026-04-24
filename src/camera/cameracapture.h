#pragma once

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
    CameraCapture(const QCameraDevice &device, tbb::flow::async_node<tbb::flow::continue_msg, FramePtr>::gateway_type &gateway, QObject*parent = nullptr);
    QVideoSink *videoSink();
    QCameraDevice cameraDevice() const;

public slots:
    void init();

signals:
    void errorOccurred(QCamera::Error error, const QString &errorString);

private:
    QCamera *m_camera;
    QCameraDevice m_cameraDevice;
    QMediaCaptureSession *m_captureSession;

    tbb::flow::async_node<tbb::flow::continue_msg, FramePtr>::gateway_type &m_gateway;
};

} // namespace MTGS