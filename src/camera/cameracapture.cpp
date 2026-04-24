#include "cameracapture.h"

namespace MTGS {

namespace tflow = tbb::flow;

CameraCapture::CameraCapture(const QCameraDevice &device, tflow::async_node<tflow::continue_msg, FramePtr>::gateway_type &gateway, QObject* parent) 
    : QObject(parent), m_cameraDevice(device), m_gateway(gateway)
{}

QVideoSink *CameraCapture::videoSink()
{
    return m_captureSession->videoSink();
}

QCameraDevice CameraCapture::cameraDevice() const
{
    return m_cameraDevice;
}

void CameraCapture::init()
{
    m_camera = new QCamera(m_cameraDevice, this);
    m_captureSession = new QMediaCaptureSession(this);
    m_captureSession->setCamera(m_camera);
    m_captureSession->setVideoSink(new QVideoSink(this));

    connect(m_captureSession->videoSink(), &QVideoSink::videoFrameChanged, 
    [this](const QVideoFrame &frame) {        
        QImage img = frame.toImage().convertToFormat(QImage::Format_BGR888);
        cv::Mat mat(img.height(), img.width(), CV_8UC3, const_cast<uchar*>(img.bits()), img.bytesPerLine());

        FramePtr f(new Frame{});
        f->mat = mat.clone();
        f->cameraId = m_cameraDevice.id();
        f->timestamp = QTime::currentTime();

        m_gateway.try_put(f);
    });
    connect(m_camera, &QCamera::errorOccurred, this, &CameraCapture::errorOccurred);

    m_camera->start();
}

}

#include "moc_cameracapture.cpp"