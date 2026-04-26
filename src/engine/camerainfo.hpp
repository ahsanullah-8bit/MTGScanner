#pragma once

#include <cstddef>
#include <tbb_patched.hpp>

#include <QPair>
#include <QThread>
#include <QVideoSink>
#include <QCameraDevice>
#include <QAtomicInteger>

#include <core/frame.hpp>

namespace MTGS {

struct CameraInfo {
    struct {
        QThread* thread = nullptr;
        class CameraCapture* worker = nullptr;
    } capture;
    QCameraDevice cameraDevice;
    QVideoSink* outVideoSink;
    QSharedPointer<tf::async_node<tf::continue_msg, FramePtr>> asyncSrc;
    QSharedPointer<tf::limiter_node<FramePtr>> preLimiter;
    QSharedPointer<tf::sequencer_node<FramePtr>> postSequencer;
    size_t skippedFramesCount = 0;
};

} // namespace MTGS
