#include "cameracapture.h"
#include <qloggingcategory.h>

namespace MTGS {

namespace tf = tbb::flow;

Q_STATIC_LOGGING_CATEGORY(capture_logger, "mtgs.capture")
CameraCapture::CameraCapture(const QString &channelId,
                            const QString &cameraId,
                            FramesPerSecond &fps,
                            QObject* parent)
    : QObject(parent)
    , m_channelId(channelId)
    , m_cameraId(cameraId)
    , m_fps(fps)
{}

void CameraCapture::setGateway(tf::async_node<tf::continue_msg, FramePtr>::gateway_type *gateway)
{
    m_gateway = gateway;
}

void CameraCapture::onVideoFrameChanged(const QVideoFrame &frame)
{
    if (!frame.isValid())
        return;

    QImage img = frame.toImage().convertToFormat(QImage::Format_BGR888);
    cv::Mat mat(img.height(), img.width(), CV_8UC3, const_cast<uchar*>(img.bits()), img.bytesPerLine());

    FramePtr f(new Frame{});
    f->mat = mat.clone();
    f->sequenceId = m_frameSequenceCount++;
    f->channelId = m_channelId;
    f->cameraId = m_cameraId;
    f->timestamp = QTime::currentTime();

    if (m_gateway)
        m_gateway->try_put(f);
    m_fps.update();
}

void CameraCapture::onErrorOccurred(const QString &channelName, QCamera::Error error, const QString &errorString)
{
    qCCritical(capture_logger) << "Channel" << channelName << error << errorString;
}

void CameraCapture::onActiveChanged(const QString &channelName, bool isActive)
{
    qCDebug(capture_logger) << "Channel" << channelName << "isActive:" << isActive;
}

}