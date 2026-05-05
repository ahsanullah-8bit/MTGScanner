#include <algorithm>
#include <tuple>

#include <QList>
#include <QUuid>
#include <QThread>
#include <QScreen>
#include <QVariant>
#include <QSettings>
#include <QQmlEngine>
#include <QMetaObject>
#include <QCameraDevice>
#include <QMediaDevices>
#include <QGuiApplication>
#include <QCoreApplication>
#include <QLoggingCategory>

#include <core/frame.hpp>
#include "engine.h"

namespace MTGS {

namespace tf = tbb::flow;

Q_STATIC_LOGGING_CATEGORY(worker_logger, "mtgs.engine.worker")

EngineWorker::EngineWorker(tbb::concurrent_unordered_map<QString, QSharedPointer<ChannelInfo>> &channels, QObject *parent)
    : QObject(parent), m_channels(channels) 
{}

EngineWorker::~EngineWorker() 
{
    // Close all cameras
    for (auto &[_, channel] : m_channels) {
        channel->capture.thread->quit();
        channel->capture.thread->wait();
    }
    // Close the graph
    m_graph.wait_for_all();

    // Save the channel configs to settings.
    saveToSettings();
}

void EngineWorker::init() 
{
    auto processor_body = [this](const FramePtr &frame, auto &ports) {
        // Process the image. For now, we just pass it through.

        if (frame->timestamp.msecsTo(QTime::currentTime()) > 100) {
            auto &info = m_channels.at(frame->channelId);
            qCWarning(worker_logger) << QString("Channel %1 frame at %2 expired")
                .arg(info->channelOptions.name)
                .arg(frame->originalFrame.startTime());

                info->totalSkippedFrames++;
                info->skippedFps.update();
                info->metrics->setSkippedFps(info->skippedFps.fps());

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

        auto &channel = m_channels.at(frame->channelId);
        channel->postSequencer->try_put(frame);
        channel->preLimiter->decrementer().try_put(tf::continue_msg());
        std::get<0>(ports).try_put(tf::continue_msg());
    };

    auto ui_notifier_body = [this](const FramePtr &f) {
        f->originalFrame = QVideoFrame(QImage(f->mat.data, f->mat.cols, f->mat.rows, f->mat.step, QImage::Format_BGR888).copy());
        
        QMetaObject::invokeMethod(this, "sendFrameToMainThread", Qt::AutoConnection, Q_ARG(FramePtr, f));
        auto &channel = m_channels.at(f->channelId);
        channel->fps.update();
        channel->metrics->setFps(channel->fps.fps());
    };

    // The critical processor node.
    m_processor = QSharedPointer<tf::multifunction_node<FramePtr, std::tuple<FramePtr, tf::continue_msg>>>::create(m_graph, tf::unlimited, processor_body);
    m_frameDistributor = QSharedPointer<tf::multifunction_node<FramePtr, std::tuple<tf::continue_msg>>>::create(m_graph, tf::unlimited, frame_distributor_body);
    m_uiNotifier = QSharedPointer<tf::function_node<FramePtr>>::create(m_graph, tf::serial, ui_notifier_body);

    tf::make_edge(tf::output_port<0>(*m_processor), *m_frameDistributor);

    // Load the channel settings
    loadFromSettings();

    emit engineLoaded();
}


void EngineWorker::saveToSettings()
{
    QSettings settings;
    settings.beginWriteArray("channels");

    size_t i = 0;
    for (auto it = m_channels.cbegin(); it != m_channels.cend(); ++it) {
        const auto& options = it->second->channelOptions;
        settings.setArrayIndex(i++);
        settings.setValue("id", options.id);
        settings.setValue("name", options.name);
        settings.setValue("maxInFlight", options.maxInFlight);
        settings.setValue("detThresh", options.detectionThreshold);
        settings.setValue("filters", QVariant::fromValue(options.filters));
        settings.setValue("cameraDeviceId", options.cameraDevice.id());
        settings.setValue("cameraDeviceDesc", options.cameraDevice.description());
        settings.setValue("winName", options.windowName);
        settings.setValue("winGeometry", options.windowGeometry);
        settings.setValue("screenSerialNo", options.screenSerialNo);
        settings.setValue("screenName", options.screenName);
    }

    settings.endArray();
}

void EngineWorker::loadFromSettings()
{
    QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    QList<QScreen*> screens = QGuiApplication::screens();

    QSettings settings;
    auto size = settings.beginReadArray("channels");
    for(decltype(size) i = 0; i < size; ++i) {
        settings.setArrayIndex(i);

        ChannelOptions options;
        options.id = settings.value("id").toString();
        options.name = settings.value("name").toString();
        options.maxInFlight = settings.value("maxInFlight").toInt();
        options.detectionThreshold = settings.value("detThresh").toFloat();
        options.filters = settings.value("filters").toStringList();
        options.windowName = settings.value("winName").toString();
        options.windowGeometry = settings.value("winGeometry").toRect();

        // Verify CameraDevice
        const QString camera_id = settings.value("cameraDeviceId").toString();
        const QString camera_desc = settings.value("cameraDeviceDesc").toString();
        const auto cam_it = std::find_if(cameras.begin(), cameras.end(),
            [&camera_id, &camera_desc] (const QCameraDevice &camera) {
                return camera.id() == camera_id || camera.description() == camera_desc;
            }
        );

        if (cam_it != cameras.end())
            options.cameraDevice = *cam_it;

        // Verify Screen
        options.screenSerialNo = settings.value("screenSerialNo").toString();
        options.screenName = settings.value("screenName").toString();
        const auto screen_it = std::find_if(screens.begin(), screens.end(),
            [&options] (QScreen *s) {
                return s->serialNumber() == options.screenSerialNo || s->name() == options.screenName;
            }
        );

        QScreen *screen = nullptr;
        if (screen_it == screens.end()) {
            // Launch the window in the primary screen.
            screen = QGuiApplication::primaryScreen();
            options.screenSerialNo = screen->serialNumber();
            options.screenName = screen->name();
            addChannel(options, nullptr, 16, screen);
        } else {
            screen = *screen_it;
        }

        // Add the channel
        addChannel(options, nullptr, 16, *screen_it);
    }

    settings.endArray();
}

void EngineWorker::addChannel(const ChannelOptions &channelOptions, QVideoSink *videoSink, int maxInFlight, QScreen *screen)
{
    if (!channelOptions.isValid()) {
        return;
    }

    auto async_src_body = [] (const tf::continue_msg &, tf::async_node<tf::continue_msg, FramePtr>::gateway_type& gateway) {};
    auto sequencer_body = [] (const FramePtr &f) { return f->sequenceId; };

    QSharedPointer<ChannelInfo> channel = QSharedPointer<ChannelInfo>::create();
    channel->asyncSrc = QSharedPointer<tf::async_node<tf::continue_msg, FramePtr>>::create(m_graph, tf::unlimited, async_src_body);
    channel->preLimiter = QSharedPointer<tf::limiter_node<FramePtr>>::create(m_graph, maxInFlight);
    channel->postSequencer = QSharedPointer<tf::sequencer_node<FramePtr>>::create(m_graph, sequencer_body);
    channel->outVideoSink = videoSink;
    channel->channelOptions = channelOptions;
    channel->metrics = QSharedPointer<ChannelMetrics>::create();
    // QMLEngine be mad, if we don't do this.
    channel->metrics->moveToThread(QCoreApplication::instance()->thread());

    tf::make_edge(*channel->asyncSrc, *channel->preLimiter);
    tf::make_edge(*channel->preLimiter, *m_processor);
    tf::make_edge(*channel->postSequencer, *m_uiNotifier);

    auto thread = new QThread();
    auto worker = new CameraCapture(channelOptions.id, channelOptions.cameraDevice, channel->asyncSrc->gateway());
    worker->moveToThread(thread);
    connect(thread, &QThread::started, worker, &CameraCapture::init);
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    connect(worker, &CameraCapture::errorOccurred, [](QCamera::Error error, const QString &errorStr) { qCritical() << error << errorStr; } );
    
    channel->capture = { thread, worker };
    channel->capture.thread->start();

    channel->metrics->setStatus(Engine::Running);
    channel->fps.start();
    channel->skippedFps.start();

    m_channels.emplace(channelOptions.id, channel);

    // TODO: Launch and link output window.

    emit channelAdded(channelOptions);
}

void EngineWorker::deleteChannel(const ChannelOptions &options)
{
    auto &info = m_channels.at(options.id);

    // Signal channel-stopped to the m_channel readers.
    info->metrics->setStatus(Engine::Stopping);

    // Stop capture
    QMetaObject::invokeMethod(info->capture.worker, "stop");
    info->capture.thread->quit();
    info->capture.thread->wait(); // TODO: Maybe add a timeout and then force quit.

    info->metrics->setStatus(Engine::Stopped);

    // Disconnect
    tf::remove_edge(*info->asyncSrc, *info->preLimiter);
    tf::remove_edge(*info->preLimiter, *m_processor);
    tf::remove_edge(*info->postSequencer, *m_uiNotifier);

    info->outVideoSink = nullptr;
    info->metrics->setStatus(Engine::Uknown);

    // Delete
    // TODO: Make sure everything dies before release.
    m_channels.unsafe_erase(options.id);

    emit channelDeleted(options);
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
    connect(worker, &EngineWorker::channelDeleted, this, &Engine::channelDeleted);

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
    connect(m_engine.worker, &EngineWorker::channelDeleted, model.data(), &ChannelModel::channelDeleted);

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
    return m_channels.at(channelId)->channelOptions;
}

QObject *Engine::channelMetrics(const QString &channelId) const
{
    if (!m_channels.contains(channelId)) {
        qCCritical(worker_logger) << QString("Requsted metrics for a non existent channel using id %1.").arg(channelId);
        return nullptr;
    }

    QObject *metrics = m_channels.at(channelId)->metrics.get();
    QQmlEngine::setObjectOwnership(metrics, QQmlEngine::CppOwnership);
    return metrics;
}

void Engine::receiveFrameNotification(const FramePtr& frame) 
{
    if (!m_channels.contains(frame->channelId)) {
        qCDebug(engine_logger) << QString("Stranded frame %1 from channel %2").arg(frame->sequenceId).arg(frame->channelId);
        return;
    }

    auto videoSink = m_channels.at(frame->channelId)->outVideoSink;
    if (videoSink)
        videoSink->setVideoFrame(frame->originalFrame);
}

void Engine::addChannel(const ChannelOptions &channelOptions, QVideoSink *videoSink, int maxInFlight, QScreen *screen)
{
    if (m_engine.thread && m_engine.worker) {
        QMetaObject::invokeMethod(m_engine.worker, "addChannel", Qt::AutoConnection,
            Q_ARG(ChannelOptions, channelOptions),
            Q_ARG(QVideoSink*, videoSink),
            Q_ARG(int, maxInFlight),
            Q_ARG(QScreen*, screen)
        );
    } else {
        qCCritical(engine_logger) << "Worker thread not initialized yet. Cannot add channel" << channelOptions.name;
    }
}

void Engine::deleteChannel(const ChannelOptions &options)
{
    if (!m_channels.contains(options.id)) {
        qCCritical(worker_logger) << QString("Trying to delete a non existent channel using id %1.").arg(options.id);
        return;
    }

    QMetaObject::invokeMethod(m_engine.worker, "deleteChannel", Q_ARG(ChannelOptions, options));
}

void Engine::registerChannelOutSink(const QString &channelId, QVideoSink *videoSink)
{
    if (!m_channels.contains(channelId)) {
        qCDebug(engine_logger) << QString("Trying to register videosink for unknown channel %1").arg(channelId);
        return;
    }

    m_channels.at(channelId)->outVideoSink = videoSink;
}

void Engine::setEngineLoaded(bool loaded) 
{
    if (m_engineLoaded == loaded)
        return;

    m_engineLoaded = loaded;
    emit engineLoaded(loaded);
}

} // namespace MTGS