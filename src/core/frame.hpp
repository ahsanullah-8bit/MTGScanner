#pragma once

#include <cstddef>

#include <QTime>
#include <QString>
#include <QObject>
#include <QVideoFrame>

#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>

#include <core/prediction.hpp>

namespace MTGS {
    
struct Frame {
    size_t sequenceId;
    QString channelId;
    QString cameraId;
    QTime timestamp;

    cv::Mat mat;
    QList<Prediction> predictions;
    QVideoFrame originalFrame;
};

using FramePtr = QSharedPointer<Frame>;

}