#pragma once

#include <tbb_patched.hpp>

#include <QThread>
#include <QAtomicInteger>
#include <QSharedPointer>

#include <core/frame.hpp>
#include <engine/cardprocessor.h>
#include <framespersecond.hpp>
#include <channeloptions.hpp>

namespace MTGS {

// This lives on the EngineWorker
struct ChannelRaw {
    struct {
        QThread* thread = nullptr;
        class CameraCapture* worker = nullptr;
    } capture;
    ChannelOptions options;

    QSharedPointer<tf::async_node<tf::continue_msg, FramePtr>> asyncSrc;
    QSharedPointer<tf::limiter_node<FramePtr>> preLimiter;
    QSharedPointer<tf::sequencer_node<FramePtr>> postSequencer;

    QSharedPointer<CardProcessor> cardProcessor;

    FramesPerSecond fps;
    FramesPerSecond skippedFps;
    FramesPerSecond captureFps;

    QAtomicInt status = 0;
    QAtomicInt visibleCards;
};

}