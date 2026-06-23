#pragma once

#include <cstddef>
#include <optional>

#include <QList>

#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>

namespace MTGS {

struct KeyPoint {
    cv::Point2f pt;
    float visibility;
    int id;
};

struct Prediction {
    cv::Rect box;
    QList<KeyPoint> keypoints;
    float confidence;
    QString className;
    int classId;
    size_t trackerId = -1;
    
    std::optional<QList<cv::Mat>> crops;
    std::optional<QList<Prediction>> subPredictions;
};

}