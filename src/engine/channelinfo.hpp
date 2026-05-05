#pragma once

#include <tbb_patched.hpp>

#include <QPair>
#include <QThread>
#include <QVideoSink>
#include <QCameraDevice>
#include <QAtomicInteger>

#include <core/frame.hpp>
#include <engine/channelmetrics.h>
#include <channeloptions.hpp>

namespace MTGS {

struct ChannelInfo {
    struct {
        QThread* thread = nullptr;
        class CameraCapture* worker = nullptr;
    } capture;
    QVideoSink* outVideoSink;
    QSharedPointer<tf::async_node<tf::continue_msg, FramePtr>> asyncSrc;
    QSharedPointer<tf::limiter_node<FramePtr>> preLimiter;
    QSharedPointer<tf::sequencer_node<FramePtr>> postSequencer;

    ChannelOptions channelOptions;
    QSharedPointer<ChannelMetrics> metrics;

    // This helps in sequencing.
    QAtomicInt skippedFrames = 0;
};

} // namespace MTGS
