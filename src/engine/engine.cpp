#include <qnamespace.h>
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

using accessor = tbb::concurrent_hash_map<QString, QSharedPointer<ChannelRaw>>::accessor;
using const_accessor = tbb::concurrent_hash_map<QString, QSharedPointer<ChannelRaw>>::const_accessor;

// ------------------ Engine Implementation ------------------
Q_STATIC_LOGGING_CATEGORY(engine_logger, "mtgs.engine")
Engine::Engine(QObject *parent)
    : QObject(parent)
    , m_availableCamerasModel(new AvailableCamerasModel(this))
    , m_cameraMngr(new CameraManager(this))
    , m_metricsTimer(new QTimer(this))
    , m_channelsModel(new ChannelModel(this))
{
    initializeGraph();

    // Connect CameraManager to the AvailableCamerasModel
    m_availableCamerasModel->setCameraManager(m_cameraMngr);

    // Connect channels model
    connect(this, &Engine::channelAdded, m_channelsModel, &ChannelModel::channelAdded);
    connect(this, &Engine::channelAboutToBeDeleted, m_channelsModel, &ChannelModel::channelDeleted);

    // Load the saved channels
    loadFromSettings();

    m_metricsTimer->start(1000);
}

Engine::~Engine() 
{
    for (auto it = m_channels.begin(); it != m_channels.end(); ++it) {
        it.value()->camera()->stop();
    }

    for (auto &[_, raw_channel] : m_rawChannels) {
        raw_channel->capture.thread->quit();
        raw_channel->capture.thread->wait();
    }

    m_graph.wait_for_all();

    saveToSettings();
}

void Engine::initializeGraph()
{
    auto processor_body = [this](const FramePtr &frame, auto &ports) {
        // Process the image. For now, we just pass it through.

        if (frame->timestamp.msecsTo(QTime::currentTime()) > 100) {
            accessor a;
            if (m_rawChannels.find(a, frame->channelId)
                && !a.empty()) {
                auto &channel = a->second;
                qCWarning(engine_logger) << QString("Channel %1 frame at %2 expired")
                    .arg(channel->options.name)
                    .arg(frame->originalFrame.startTime());

                channel->totalSkippedFrames++;
                channel->skippedFps.update();
            }

            std::get<1>(ports).try_put(tf::continue_msg());
            return;
        }

        std::get<0>(ports).try_put(frame);
        std::get<1>(ports).try_put(tf::continue_msg());
    };

    auto frame_distributor_body = [this](const FramePtr &frame, auto &ports) {
        accessor a;
        if (!m_rawChannels.find(a, frame->channelId)
            || a.empty()) {
            std::get<0>(ports).try_put(tf::continue_msg());
            return;
        }

        // TODO: Maybe check for frame expiration here as well.

        auto &channel = a->second;
        channel->postSequencer->try_put(frame);
        channel->preLimiter->decrementer().try_put(tf::continue_msg());
        std::get<0>(ports).try_put(tf::continue_msg());
    };

    auto ui_notifier_body = [this](const FramePtr &f) {
        QImage img(f->mat.data, f->mat.cols, f->mat.rows, f->mat.step, QImage::Format_BGR888);
        f->originalFrame = QVideoFrame(img.copy());
        
        QMetaObject::invokeMethod(this, "sendFrameToMainThread", Qt::QueuedConnection, Q_ARG(FramePtr, f));

        accessor a;
        if (m_rawChannels.find(a, f->channelId) && !a.empty()) {
            auto &channel = a->second;
            channel->fps.update();
        }
    };

    // The critical processor node.
    m_processor = QSharedPointer<tf::multifunction_node<FramePtr, std::tuple<FramePtr, tf::continue_msg>>>::create(m_graph, tf::unlimited, processor_body);
    m_frameDistributor = QSharedPointer<tf::multifunction_node<FramePtr, std::tuple<tf::continue_msg>>>::create(m_graph, tf::unlimited, frame_distributor_body);
    m_uiNotifier = QSharedPointer<tf::function_node<FramePtr>>::create(m_graph, tf::serial, ui_notifier_body);

    tf::make_edge(tf::output_port<0>(*m_processor), *m_frameDistributor);
}

ChannelModel* Engine::channelsModel() const
{
    return m_channelsModel;
}

AvailableCamerasModel* Engine::availableCamerasModel() const
{
    return m_availableCamerasModel;
}

Channel *Engine::createChannel()
{
    Channel *channel = new Channel(this);
    channel->options().id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    channel->setCamera(new QCamera(channel));
    channel->setCaptureSession(new QMediaCaptureSession(channel));
    channel->captureSession()->setCamera(channel->camera());
    channel->setMetrics(new ChannelMetrics(channel));

    QQmlEngine::setObjectOwnership(channel, QQmlEngine::CppOwnership);

    // Debug channel destruction
#ifndef NDEBUG
    connect(channel, &Channel::destroyed, [id = channel->options().id] () {
        qCCritical(engine_logger) << "Channel got destroyed, named" << id;
    });
#endif

    return channel;
}

Channel *Engine::channel(const QString &channelId)
{
    if (!m_channels.contains(channelId)) {
        qCCritical(engine_logger) << QString("Requsted channel for a non existent id %1.").arg(channelId);
        return nullptr;
    }

    Channel *channel = m_channels.value(channelId);
    QQmlEngine::setObjectOwnership(channel, QQmlEngine::CppOwnership);
    return channel;
}

Channel *Engine::channelAtIndex(int index)
{
    if (index < 0 || index >= m_channelIdIndexMap.size()) {
        qCCritical(engine_logger) << QString("Requsted channel for a non existent index %1.").arg(index);
        return nullptr;
    }

    return channel(m_channelIdIndexMap.at(index));
}

void Engine::destroyChannel(Channel *channel)
{
    if (!channel)
        return;

    qCCritical(engine_logger) << "Destroy channel called for" << qobject_cast<Channel*>(channel)->options().id;

    channel->deleteLater();
}

bool Engine::channelExists(const QString &id)
{
    return m_channels.contains(id);
}

CameraManager *Engine::cameraManager()
{
    return m_cameraMngr;
}

void Engine::saveToSettings()
{
    QSettings settings;
    settings.beginWriteArray("channels");

    size_t i = 0;
    for (auto it = m_channels.cbegin(); it != m_channels.cend(); ++it) {
        const auto& options = it.value()->options();
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
        settings.setValue("status", it.value()->metrics()->status());
    }

    settings.endArray();
}

void Engine::loadFromSettings()
{
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
        int channelStatus = settings.value("status").toInt();

        // Verify CameraDevice
        const QString camera_id = settings.value("cameraDeviceId").toString();
        const QString camera_desc = settings.value("cameraDeviceDesc").toString();
        auto camera_opt = m_cameraMngr->findBy(camera_id, camera_desc);

        // Verify Screen
        options.screenSerialNo = settings.value("screenSerialNo").toString();
        options.screenName = settings.value("screenName").toString();
        const auto screen_it = std::find_if(screens.begin(), screens.end(),
            [&options] (QScreen *s) {
                return s->serialNumber() == options.screenSerialNo || s->name() == options.screenName;
            }
        );

        // Add the channel
        Channel *channel = createChannel();

        if (camera_opt) {
            options.cameraDevice = camera_opt.value();
            channel->camera()->setCameraDevice(options.cameraDevice);
        } else {
            channelStatus = ChannelStatus::Stopped;
        }

        QScreen *screen = nullptr;
        if (screen_it == screens.end()) {
            // Launch the window in the primary screen.
            screen = QGuiApplication::primaryScreen();
            options.screenSerialNo = screen->serialNumber();
            options.screenName = screen->name();
        } else {
            screen = *screen_it;
        }

        channel->setOptions(options);
        addChannel(channel, channelStatus, screen);
    }

    settings.endArray();
}

void Engine::receiveFrameNotification(const FramePtr& frame)
{
    if (!m_channels.contains(frame->channelId)) {
        qCDebug(engine_logger) << QString("Stranded frame %1 from channel %2").arg(frame->sequenceId).arg(frame->channelId);
        return;
    }

    auto videoSink = m_channels.value(frame->channelId)->outVideoSink();
    if (videoSink)
        videoSink->setVideoFrame(frame->originalFrame);
}

void Engine::addChannel(Channel *channel, int status, QScreen *screen)
{
    if (!channel)
        return;

    QSharedPointer<ChannelRaw> channel_raw(new ChannelRaw);

    channel->camera()->stop();
    channel->captureSession()->setVideoOutput(nullptr);
    channel->captureSession()->setVideoSink(new QVideoSink(channel->captureSession()));

    // Bind metrics
    channel->metrics()->setFps(&channel_raw->fps);
    channel->metrics()->setCaptureFps(&channel_raw->captureFps);
    channel->metrics()->setSkippedFps(&channel_raw->skippedFps);
    channel->metrics()->setStatus(&channel_raw->status);
    channel->metrics()->setVisibleCards(&channel_raw->visibleCards);
    connect(m_metricsTimer, &QTimer::timeout, channel->metrics(), &ChannelMetrics::fireMetricsUpdate);

    // Per-channel nodes and edges
    auto async_src_body = [] (const tf::continue_msg &, tf::async_node<tf::continue_msg, FramePtr>::gateway_type& gateway) {};
    auto sequencer_body = [] (const FramePtr &f) { return f->sequenceId; };
    channel_raw->asyncSrc = QSharedPointer<tf::async_node<tf::continue_msg, FramePtr>>::create(m_graph, tf::unlimited, async_src_body);
    channel_raw->preLimiter = QSharedPointer<tf::limiter_node<FramePtr>>::create(m_graph, channel->options().maxInFlight);
    channel_raw->postSequencer = QSharedPointer<tf::sequencer_node<FramePtr>>::create(m_graph, sequencer_body);

    tf::make_edge(*channel_raw->asyncSrc, *channel_raw->preLimiter);
    tf::make_edge(*channel_raw->preLimiter, *m_processor);
    tf::make_edge(*channel_raw->postSequencer, *m_uiNotifier);

    // Camera Capture setup
    auto *thread = new QThread();
    auto *capture = new CameraCapture(channel->options().id, channel->options().cameraDevice.id(), channel_raw->captureFps);
    capture->setGateway(&channel_raw->asyncSrc->gateway());
    connect(thread, &QThread::finished, capture, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    connect(channel->captureSession()->videoSink(), &QVideoSink::videoFrameChanged, capture, &CameraCapture::onVideoFrameChanged);
    connect(channel->camera(), &QCamera::errorOccurred,
        [this, channel = channel, capture = capture] (QCamera::Error error, const QString &errorStr) {
            accessor a;
            if (m_rawChannels.find(a, channel->options().id) && !a.empty()) {
                auto &raw_channel = a->second;
                raw_channel->status.storeRelaxed(Engine::Errored);
            }

            QMetaObject::invokeMethod(capture,
                "onErrorOccurred",
                Q_ARG(QString, channel->options().name),
                Q_ARG(QCamera::Error, error),
                Q_ARG(QString, errorStr));
        }
    );
    connect(channel->camera(), &QCamera::activeChanged,
        [this, channel = channel, capture = capture] (bool active) {
            accessor a;
            if (m_rawChannels.find(a, channel->options().id) && !a.empty()) {
                auto &raw_channel = a->second;
                if (raw_channel->status.loadRelaxed() == Engine::Errored) {
                    // This is an error scenario, not a common start/stop one.
                    // We let that be.
                    return;
                }
    
                if (active)
                    raw_channel->status.storeRelease(Engine::Running);
                else
                    raw_channel->status.storeRelease(Engine::Stopped);
            }

            QMetaObject::invokeMethod(capture,
                "onActiveChanged",
                Q_ARG(QString, channel->options().name),
                Q_ARG(bool, active));
        }
    );

    capture->moveToThread(thread);
    channel_raw->capture = {thread, capture};
    channel_raw->options = channel->options();
    channel_raw->status.storeRelaxed(status);
    channel_raw->capture.thread->start();
    channel_raw->skippedFps.start();
    channel_raw->fps.start();

    if (!m_rawChannels.emplace(channel->options().id, channel_raw)) {
        qCCritical(engine_logger) << "Failed to insert a new channel after connecting it, named" << channel->options().name;
        
        // Cleanup, in reverse
        disconnect(m_metricsTimer, &QTimer::timeout, channel->metrics(), &ChannelMetrics::fireMetricsUpdate);
        channel->metrics()->setFps(nullptr);
        channel->metrics()->setCaptureFps(nullptr);
        channel->metrics()->setSkippedFps(nullptr);
        channel->metrics()->setStatus(nullptr);
        channel->metrics()->setVisibleCards(nullptr);

        channel->camera()->disconnect(); // all

        capture->setGateway(nullptr);

        thread->quit();
        thread->wait();

        tf::remove_edge(*channel_raw->preLimiter, *m_processor);
        tf::remove_edge(*channel_raw->asyncSrc,   *channel_raw->preLimiter);
        tf::remove_edge(*channel_raw->postSequencer, *m_uiNotifier);

        channel->captureSession()->setVideoSink(nullptr);
        channel->deleteLater();
        return;
    }

    // Mark the camera
    m_cameraMngr->setCameraInUse(channel->options().cameraDevice, true);
    if (channel->metrics()->status() != Engine::Stopped) {
        // Then it's probably Unknown
        channel->camera()->start();
    }

    m_channels.emplace(channel->options().id, channel);
    m_channelIdIndexMap.append(channel->options().id);
    emit channelAdded(channel->options());
}

void Engine::deleteChannel(const ChannelOptions &options)
{
    if (!m_channels.contains(options.id)) {
        qCCritical(engine_logger) << QString("Trying to delete a non existent channel using id %1.").arg(options.id);
        return;
    }

    // In case someone wants to opt-out
    emit channelAboutToBeDeleted(options);

    // Cleanup
    auto *channel = m_channels.value(options.id);
    disconnect(m_metricsTimer, &QTimer::timeout, channel->metrics(), &ChannelMetrics::fireMetricsUpdate);
    channel->camera()->stop();
    channel->metrics()->setFps(nullptr);
    channel->metrics()->setCaptureFps(nullptr);
    channel->metrics()->setSkippedFps(nullptr);
    channel->metrics()->setStatus(nullptr);
    channel->metrics()->setVisibleCards(nullptr);
    
    channel->camera()->disconnect(); // all

    accessor a;
    if (!m_rawChannels.find(a, options.id)
        || a.empty()) {
        qCCritical(engine_logger) << "Failed to delete a channel named" << options.name;
        return;
    }
    auto &raw_channel = a->second;
    raw_channel->capture.worker->setGateway(nullptr);
    raw_channel->capture.thread->quit();
    raw_channel->capture.thread->wait(); // TODO: Maybe add a timeout and then force quit.
    raw_channel->status.storeRelaxed(Engine::Unknown);

    tf::remove_edge(*raw_channel->preLimiter, *m_processor);
    tf::remove_edge(*raw_channel->asyncSrc,   *raw_channel->preLimiter);
    tf::remove_edge(*raw_channel->postSequencer, *m_uiNotifier);

    channel->captureSession()->setVideoSink(nullptr);

    // Unmark the camera on destruction of channel.
    connect(channel, &Channel::destroyed, 
        [this, device = options.cameraDevice] () {
            m_cameraMngr->setCameraInUse(device, false);
        }
    );

    // Delete
    m_channels.take(options.id)->deleteLater();
    m_channelIdIndexMap.removeOne(options.id);
    if (!m_rawChannels.erase(a)) {
        qCCritical(engine_logger) << "Failed to erase a channel after disconnecting it, named" << options.name;
        return;
    }

    emit channelDeleted(options);
}

void Engine::startChannel(const QString &channelId)
{
    if (!m_channels.contains(channelId)) {
        qCCritical(engine_logger) << QString("Trying to start a non existent channel using id %1.").arg(channelId);
        return;
    }

    auto channel = m_channels.value(channelId);
    if (channel->metrics()->status() != ChannelStatus::Running) {
        channel->camera()->start();
        // EngineWoker will change the status
    }
}

void Engine::stopChannel(const QString &channelId)
{
    if (!m_channels.contains(channelId)) {
        qCCritical(engine_logger) << QString("Trying to stop a non existent channel using id %1.").arg(channelId);
        return;
    }

    auto channel = m_channels.value(channelId);
    if (channel->metrics()->status() != ChannelStatus::Stopped || channel->camera()->isActive()) {
        channel->camera()->stop();
        // EngineWoker will change the status
    }
}

void Engine::registerChannelOutSink(const QString &channelId, QVideoSink *videoSink)
{
    if (!m_channels.contains(channelId)) {
        qCDebug(engine_logger) << QString("Trying to register videosink for unknown channel %1").arg(channelId);
        return;
    }

    m_channels.value(channelId)->setOutVideoSink(videoSink);
}

void Engine::unRegisterChannelOutSink(const QString &channelId)
{
    if (!m_channels.contains(channelId))
        return;

    m_channels.value(channelId)->setOutVideoSink(nullptr);
}

} // namespace MTGS