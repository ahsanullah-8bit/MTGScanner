#pragma once

#include <cstddef>
#include <optional>
#include <vector>

#include <opencv2/core/types.hpp>

namespace MTGS {

struct Prediction {
    cv::Rect box;
    std::vector<cv::Point3f> keypoints;
    float confidence;
    std::string className;
    int classId;
    size_t trackerId = -1;

    std::optional<std::vector<Prediction>> subPredictions;
};

}