#include <tuple>

#include <QUuid>
#include <QThread>
#include <QMetaObject>
#include <QMediaDevices>
#include <QLoggingCategory>

#include <core/frame.hpp>
#include "engine.h"

namespace MTGS {

namespace tf = tbb::flow;

Q_STATIC_LOGGING_CATEGORY(worker_logger, "mtgs.engine.worker")

EngineWorker::EngineWorker(tbb::concurrent_unordered_map<QString, ChannelInfo> &channels, QObject *parent)
    : QObject(parent), m_channels(channels) 
{}

EngineWorker::~EngineWorker() 
{
    // Close all cameras
    for (auto &[_, channel] : m_channels) {
        channel.capture.thread->quit();
        channel.capture.thread->wait();
    }
    // Close the graph
    m_graph.wait_for_all();
}

void EngineWorker::init() 
{
    auto processor_body = [this](const FramePtr &frame, auto &ports) {
        // Process the image. For now, we just pass it through.

        if (frame->timestamp.msecsTo(QTime::currentTime()) > 100) {
            qCWarning(worker_logger) << QString("Channel %1 frame at %2 expired")
                .arg(m_channels.at(frame->channelId).channelOptions.name)
                .arg(frame->originalFrame.startTime());

                m_channels.at(frame->channelId).skippedFramesCount++;
                std::get<1>(ports).try_put(tf::continue_msg());
                return;
        }

        std::get<0>(ports).try_put(frame);
        std::get<1>(ports).try_put(tf::continue_msg());
    };

    auto frame_distributor_body = [this](const FramePtr &frame, auto &ports) {
        if (!m_channels.contains(frame->channelId)) {
            std::get<0>(ports).try_put(tf::continue_msg());
            return;
        }

        // TODO: Maybe check for frame expiration here as well.

        m_channels.at(frame->channelId).postSequencer->try_put(frame);
        std::get<0>(ports).try_put(tf::continue_msg());
    };

    auto ui_notifier_body = [this](const FramePtr &f) {
        f->originalFrame = QVideoFrame(QImage(f->mat.data, f->mat.cols, f->mat.rows, f->mat.step, QImage::Format_BGR888).copy());
        
        QMetaObject::invokeMethod(this, "sendFrameToMainThread", Qt::AutoConnection, Q_ARG(FramePtr, f));
    };

    // The critical processor node.
    m_processor = QSharedPointer<tf::multifunction_node<FramePtr, std::tuple<FramePtr, tf::continue_msg>>>::create(m_graph, tf::unlimited, processor_body);
    m_frameDistributor = QSharedPointer<tf::multifunction_node<FramePtr, std::tuple<tf::continue_msg>>>::create(m_graph, tf::unlimited, frame_distributor_body);
    m_uiNotifier = QSharedPointer<tf::function_node<FramePtr>>::create(m_graph, tf::serial, ui_notifier_body);

    tf::make_edge(*m_processor, *m_frameDistributor);

    emit engineLoaded();
}

void EngineWorker::addChannel(const ChannelOptions &channelOptions, QVideoSink *videoSink, int maxInFlight)
{
    if (!channelOptions.isValid()) {
        return;
    }

    auto async_src_body = [] (const tf::continue_msg &, tf::async_node<tf::continue_msg, FramePtr>::gateway_type& gateway) {};
    auto sequencer_body = [] (const FramePtr &f) { return f->sequenceId; };

    ChannelInfo channel;
    channel.asyncSrc = QSharedPointer<tf::async_node<tf::continue_msg, FramePtr>>::create(m_graph, tf::unlimited, async_src_body);
    channel.preLimiter = QSharedPointer<tf::limiter_node<FramePtr>>::create(m_graph, maxInFlight);
    channel.postSequencer = QSharedPointer<tf::sequencer_node<FramePtr>>::create(m_graph, sequencer_body);
    channel.outVideoSink = videoSink;
    channel.channelOptions = channelOptions;

    tf::make_edge(*channel.asyncSrc, *channel.preLimiter);
    tf::make_edge(*channel.preLimiter, *m_processor);
    tf::make_edge(tf::output_port<1>(*m_processor), channel.preLimiter->decrementer());
    tf::make_edge(*channel.postSequencer, *m_uiNotifier);

    auto thread = new QThread();
    auto worker = new CameraCapture(channelOptions.id, channelOptions.cameraDevice, channel.asyncSrc->gateway());
    worker->moveToThread(thread);
    connect(thread, &QThread::started, worker, &CameraCapture::init);
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    connect(worker, &CameraCapture::errorOccurred, [](QCamera::Error error, const QString &errorStr) { qCritical() << error << errorStr; } );
    
    channel.capture = { thread, worker };
    channel.capture.thread->start();

    m_channels.emplace(channelOptions.id, channel);

    // TODO: Launch and link output window.

    emit channelAdded(channelOptions);
}

// ------------------ Engine Implementation ------------------
Q_STATIC_LOGGING_CATEGORY(engine_logger, "mtgs.engine")
Engine::Engine() 
{
    auto thread = new QThread();
    auto worker = new EngineWorker(m_channels);
    worker->moveToThread(thread);
    connect(thread, &QThread::started, worker, &EngineWorker::init);
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    connect(worker, &EngineWorker::engineLoaded, this, &Engine::setEngineLoaded);
    connect(worker, &EngineWorker::sendFrameToMainThread, this, &Engine::receiveFrameNotification);
    connect(worker, &EngineWorker::channelAdded, this, &Engine::channelAdded);

    m_engine.worker = worker;
    m_engine.thread = thread;
    thread->start();
}

Engine::~Engine() 
{
    m_engine.thread->quit();
    m_engine.thread->wait();
}

QSharedPointer<ChannelModel> Engine::createSharedChannelModel() 
{
    QSharedPointer<ChannelModel> model = QSharedPointer<ChannelModel>::create(m_channels, this);
    connect(m_engine.worker, &EngineWorker::channelAdded, model.data(), &ChannelModel::channelAdded);

    return model;
}

bool Engine::isEngineLoaded() const
{
    return m_engineLoaded;
}

QString Engine::createChannelId() const
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

ChannelOptions Engine::createChannelOptions() const {
    return ChannelOptions();
}

ChannelOptions Engine::channelOptions(const QString &channelId) const
{
    return m_channels.at(channelId).channelOptions;
}

void Engine::receiveFrameNotification(const FramePtr& frame) 
{
    if (!m_channels.contains(frame->channelId)) {
        qCDebug(engine_logger) << QString("Stranded frame %1 from channel %2").arg(frame->sequenceId).arg(frame->channelId);
        return;
    }    void channelAdded(const ChannelOptions &channelOptions);


    auto videoSink = m_channels[frame->channelId].outVideoSink;
    if (videoSink)
        videoSink->setVideoFrame(frame->originalFrame);
}

void Engine::registerChannelOutSink(const QString &channelId, QVideoSink *videoSink) 
{
    if (!m_channels.contains(channelId)) {
        qCDebug(engine_logger) << QString("Trying to register videosink for unknown channel %1").arg(channelId);
        return;
    }

    m_channels.at(channelId).outVideoSink = videoSink;
}

void Engine::addChannel(const ChannelOptions &channelOptions, QVideoSink *videoSink, int maxInFlight)
{
    if (m_engine.thread && m_engine.worker) {
        QMetaObject::invokeMethod(m_engine.worker, "addChannel", Qt::AutoConnection,
            Q_ARG(ChannelOptions, channelOptions), Q_ARG(QVideoSink*, videoSink), Q_ARG(int, maxInFlight));
    } else {
        qCCritical(engine_logger) << "Worker thread not initialized yet. Cannot add channel" << channelOptions.name;
    }
}

void Engine::setEngineLoaded(bool loaded) 
{
    if (m_engineLoaded == loaded)
        return;

    m_engineLoaded = loaded;
    emit engineLoaded(loaded);
}

} // namespace MTGS