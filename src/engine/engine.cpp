#include <algorithm>
#include <iterator>
#include <qloggingcategory.h>
#include <qobject.h>
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

Q_STATIC_LOGGING_CATEGORY(worker_logger, "mtgs.engine.worker")

EngineWorker::EngineWorker(QObject *parent)
    : QObject(parent)
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
}

void EngineWorker::init() 
{
    auto processor_body = [this](const FramePtr &frame, auto &ports) {
        // Process the image. For now, we just pass it through.

        if (frame->timestamp.msecsTo(QTime::currentTime()) > 100) {
            accessor a;
            if (m_channels.find(a, frame->channelId)
                && !a.empty()) {
                auto &info = a->second;
                qCWarning(worker_logger) << QString("Channel %1 frame at %2 expired")
                    .arg(info->options.name)
                    .arg(frame->originalFrame.startTime());

                info->totalSkippedFrames++;
                info->skippedFps.update();
            }

            std::get<1>(ports).try_put(tf::continue_msg());
            return;
        }

        std::get<0>(ports).try_put(frame);
        std::get<1>(ports).try_put(tf::continue_msg());
    };

    auto frame_distributor_body = [this](const FramePtr &frame, auto &ports) {
        accessor a;
        if (!m_channels.find(a, frame->channelId)
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
        
        QMetaObject::invokeMethod(this, "sendFrameToMainThread", Qt::AutoConnection, Q_ARG(FramePtr, f));

        accessor a;
        if (m_channels.find(a, f->channelId) && !a.empty()) {
            auto &channel = a->second;
            channel->fps.update();
        }
    };

    // The critical processor node.
    m_processor = QSharedPointer<tf::multifunction_node<FramePtr, std::tuple<FramePtr, tf::continue_msg>>>::create(m_graph, tf::unlimited, processor_body);
    m_frameDistributor = QSharedPointer<tf::multifunction_node<FramePtr, std::tuple<tf::continue_msg>>>::create(m_graph, tf::unlimited, frame_distributor_body);
    m_uiNotifier = QSharedPointer<tf::function_node<FramePtr>>::create(m_graph, tf::serial, ui_notifier_body);

    tf::make_edge(tf::output_port<0>(*m_processor), *m_frameDistributor);

    emit engineLoaded();
}

void EngineWorker::addChannel(QSharedPointer<ChannelRaw> channel)
{
    auto async_src_body = [] (const tf::continue_msg &, tf::async_node<tf::continue_msg, FramePtr>::gateway_type& gateway) {};
    auto sequencer_body = [] (const FramePtr &f) { return f->sequenceId; };

    channel->asyncSrc = QSharedPointer<tf::async_node<tf::continue_msg, FramePtr>>::create(m_graph, tf::unlimited, async_src_body);
    channel->preLimiter = QSharedPointer<tf::limiter_node<FramePtr>>::create(m_graph, channel->options.maxInFlight);
    channel->postSequencer = QSharedPointer<tf::sequencer_node<FramePtr>>::create(m_graph, sequencer_body);
    channel->capture.worker->setGateway(&channel->asyncSrc->gateway());

    tf::make_edge(*channel->asyncSrc, *channel->preLimiter);
    tf::make_edge(*channel->preLimiter, *m_processor);
    tf::make_edge(*channel->postSequencer, *m_uiNotifier);

    channel->capture.thread->start();
    channel->fps.start();
    channel->skippedFps.start();

    if (!m_channels.emplace(channel->options.id, channel)) {
        qCCritical(worker_logger) << "Failed to insert a new channel after connecting it, named" << channel->options.name;
        // TODO: We may need to do cleanup, if this ever happens.
        return;
    }

    emit channelReady(channel->options);
}

void EngineWorker::deleteChannel(const ChannelOptions &options)
{
    accessor a;
    if (!m_channels.find(a, options.id)
        || a.empty()) {
        qCCritical(worker_logger) << "Failed to delete a channel named" << options.name;
        return;
    }

    auto &info = a->second;
    // Stop capture worker
    info->capture.thread->quit();
    info->capture.thread->wait(); // TODO: Maybe add a timeout and then force quit.

    // Disconnect
    tf::remove_edge(*info->asyncSrc, *info->preLimiter);
    tf::remove_edge(*info->preLimiter, *m_processor);
    tf::remove_edge(*info->postSequencer, *m_uiNotifier);

    info->status.storeRelaxed(Engine::Unknown);

    // Delete
    // TODO: Make sure everything dies before release.
    if (!m_channels.erase(a)) {
        qCCritical(worker_logger) << "Failed to erase a channel after disconnecting it, named" << options.name;
        return;
    }

    emit channelDeleted(options);
}

void EngineWorker::onChannelErrorOccurred(const QString &channelId, QCamera::Error error, const QString &errorStr)
{
    accessor a;
    if (!m_channels.find(a, channelId) || a.empty())
        return;

    auto &channel = a->second;
    channel->status.storeRelaxed(Engine::Errored);
    qCCritical(worker_logger) << error << errorStr;
}

void EngineWorker::onChannelActiveChanged(const QString &channelId, bool active)
{
    accessor a;
    if (!m_channels.find(a, channelId) || a.empty())
        return;

    auto &channel = a->second;
    if (channel->status.loadRelaxed() == Engine::Errored) {
        // This is an error scenario, not a common start/stop one.
        // We let that be.
        return;
    }

    if (active) {
        channel->status.storeRelease(Engine::Running);
    } else {
        channel->status.storeRelease(Engine::Stopped);
    }
    qCDebug(worker_logger) << "Channel" << channel->options.name << static_cast<Engine::ChannelStatus>(channel->status.loadRelaxed());
}


// ------------------ Engine Implementation ------------------
Q_STATIC_LOGGING_CATEGORY(engine_logger, "mtgs.engine")
Engine::Engine(QObject *parent)
    : QObject(parent)
    , m_cameraMngr(new CameraManager(this))
    , m_availableCamerasModel(new AvailableCamerasModel(this))
    , m_metricsTimer(new QTimer(this))
    , m_channelsModel(new ChannelModel(this))
{
    // Startup the EngineWorker
    auto thread = new QThread();
    auto worker = new EngineWorker();
    worker->moveToThread(thread);
    connect(thread, &QThread::started, worker, &EngineWorker::init);
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    connect(worker, &EngineWorker::engineLoaded, this, &Engine::setEngineLoaded);
    connect(worker, &EngineWorker::sendFrameToMainThread, this, &Engine::receiveFrameNotification);
    connect(worker, &EngineWorker::channelReady, this, &Engine::onChannelReady);
    connect(worker, &EngineWorker::channelDeleted, this, &Engine::onChannelDeleted);

    m_engine.worker = worker;
    m_engine.thread = thread;
    thread->start();

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
    m_engine.thread->quit();
    m_engine.thread->wait();

    saveToSettings();
}

bool Engine::isEngineLoaded() const
{
    return m_engineLoaded;
}

ChannelModel* Engine::channelsModel() const
{
    return m_channelsModel;
}

AvailableCamerasModel* Engine::availableCamerasModel() const
{
    return m_availableCamerasModel;
}

QObject *Engine::createChannel()
{
    Channel *channel = new Channel(this);
    channel->options().id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    channel->setCamera(new QCamera(channel));
    channel->setCaptureSession(new QMediaCaptureSession(channel));
    channel->captureSession()->setCamera(channel->camera());
    channel->setMetrics(new ChannelMetrics(channel));

    QQmlEngine::setObjectOwnership(channel, QQmlEngine::CppOwnership);

    connect(channel, &Channel::destroyed, [id = channel->options().id] () {
        qCCritical(engine_logger) << "Channel got destroyed, named" << id;
    });

    return channel;
}

void Engine::destroyChannel(QObject *channel)
{
    if (!channel)
        return;

    qCCritical(engine_logger) << "Destroy channel called for" << qobject_cast<Channel*>(channel)->options().id;

    channel->deleteLater();
}

QObject *Engine::channel(const QString &channelId)
{
    if (!m_channels.contains(channelId)) {
        qCCritical(engine_logger) << QString("Requsted channel for a non existent id %1.").arg(channelId);
        return nullptr;
    }

    Channel *channel = m_channels.value(channelId);
    QQmlEngine::setObjectOwnership(channel, QQmlEngine::CppOwnership);
    return channel;
}

QObject *Engine::channelAtIndex(int index)
{
    auto it = m_channels.begin();
    std::advance(it, index);
    if (it == m_channels.end()) {
        qCCritical(engine_logger) << QString("Requsted channel for a non existent index %1.").arg(index);
        return nullptr;
    }

    Channel *channel = it.value();
    QQmlEngine::setObjectOwnership(channel, QQmlEngine::CppOwnership);
    return channel;
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
        QObject *channel_obj = createChannel();
        Channel *channel = qobject_cast<Channel*>(channel_obj);

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

void Engine::addChannel(QObject *channelObj, int status, QScreen *screen)
{
    if (!channelObj)
        return;

    Channel *channel = qobject_cast<Channel*>(channelObj);
    QSharedPointer<ChannelRaw> channel_raw(new ChannelRaw);

    channel->camera()->stop();
    channel->captureSession()->setVideoOutput(nullptr);
    channel->captureSession()->setVideoSink(new QVideoSink(channel->captureSession()));
    channel->metrics()->setFps(&channel_raw->fps);
    channel->metrics()->setCaptureFps(&channel_raw->captureFps);
    channel->metrics()->setSkippedFps(&channel_raw->skippedFps);
    channel->metrics()->setStatus(&channel_raw->status);
    channel->metrics()->setVisibleCards(&channel_raw->visibleCards);

    auto *thread = new QThread();
    auto *capture = new CameraCapture(channel->options().id, channel->options().cameraDevice.id(), channel_raw->captureFps);
    connect(thread, &QThread::finished, capture, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    connect(channel->captureSession()->videoSink(), &QVideoSink::videoFrameChanged, capture, &CameraCapture::onVideoFrameChanged);
    connect(channel->camera(), &QCamera::errorOccurred,
        [this, id = channel->options().id] (QCamera::Error error, const QString &errorStr) {
            QMetaObject::invokeMethod(m_engine.worker,
                "onChannelErrorOccurred",
                Q_ARG(QString, id),
                Q_ARG(QCamera::Error, error),
                Q_ARG(QString, errorStr));
        }
    );
    connect(channel->camera(), &QCamera::activeChanged,
        [this, id = channel->options().id] (bool active) {
            QMetaObject::invokeMethod(m_engine.worker,
                "onChannelActiveChanged",
                Q_ARG(QString, id),
                Q_ARG(bool, active));
        }
    );

    capture->moveToThread(thread);
    channel_raw->capture = {thread, capture};
    channel_raw->options = channel->options();
    channel_raw->status.storeRelaxed(status);

    m_channels.emplace(channel->options().id, channel);

    QMetaObject::invokeMethod(m_engine.worker, "addChannel", Qt::AutoConnection, Q_ARG(QSharedPointer<ChannelRaw>, channel_raw));
}

void Engine::deleteChannel(const ChannelOptions &options)
{
    if (!m_channels.contains(options.id)) {
        qCCritical(worker_logger) << QString("Trying to delete a non existent channel using id %1.").arg(options.id);
        return;
    }

    // In case someone wants to opt-out
    emit channelAboutToBeDeleted(options);

    auto *channel = m_channels.value(options.id);
    channel->camera()->stop();

    // TODO: Maybe connect to the channel->camera()->activeChanged
    QMetaObject::invokeMethod(m_engine.worker, "deleteChannel", Q_ARG(ChannelOptions, options));
}

void Engine::startChannel(const QString &channelId)
{
    if (!m_channels.contains(channelId)) {
        qCCritical(worker_logger) << QString("Trying to start a non existent channel using id %1.").arg(channelId);
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
        qCCritical(worker_logger) << QString("Trying to stop a non existent channel using id %1.").arg(channelId);
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

void Engine::onChannelReady(const ChannelOptions &options)
{
    if (!m_channels.contains(options.id)) {
        qCCritical(worker_logger) << "Failed to start camera after addition for channel" << options.name;
        return;
    }

    // Mark the camera
    m_cameraMngr->setCameraInUse(options.cameraDevice, true);

    auto channel = m_channels.value(options.id);
    if (channel->metrics()->status() != Engine::Stopped) {
        // Then it's probably Unknown
        // channel->camera()->start();
    }

    connect(m_metricsTimer, &QTimer::timeout, channel->metrics(), &ChannelMetrics::fireMetricsUpdate);
    emit channelAdded(options);
}

void Engine::onChannelDeleted(const ChannelOptions &options)
{
    Channel *channel = m_channels.take(options.id);
    if (!channel) return;

    // Mark the camera
    connect(channel, &Channel::destroyed, 
        [this, device = options.cameraDevice] () {
            m_cameraMngr->setCameraInUse(device, false);
        }
    );
    channel->deleteLater();

    emit channelDeleted(options);
}

void Engine::setEngineLoaded(bool loaded) 
{
    if (m_engineLoaded == loaded)
        return;

    m_engineLoaded = loaded;
    emit engineLoaded(loaded);
}

} // namespace MTGS