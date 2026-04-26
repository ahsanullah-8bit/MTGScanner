
#include <tuple>

#include <QThread>
#include <QMetaObject>
#include <QMediaDevices>
#include <QLoggingCategory>

#include <core/frame.hpp>
#include "engine.h"

namespace MTGS {

namespace tf = tbb::flow;

Q_STATIC_LOGGING_CATEGORY(worker_logger, "mtgs.engine.worker")

EngineWorker::EngineWorker(tbb::concurrent_unordered_map<QString, CameraInfo> &cameras, QObject *parent)
    : QObject(parent), m_cameras(cameras) 
{}

EngineWorker::~EngineWorker() 
{
    // Close all cameras

    m_graph.wait_for_all();
}

void EngineWorker::init() 
{
    auto processor_body = [this](const FramePtr &frame, auto &ports) {
        // Process the image. For now, we just pass it through.

        if (frame->timestamp.msecsTo(QTime::currentTime()) > 100) {
            qCWarning(worker_logger) << QString("Cam %1 frame at %2 expired")
                .arg(m_cameras.at(frame->cameraId).cameraDevice.description())
                .arg(frame->originalFrame.startTime());

                m_cameras.at(frame->cameraId).skippedFramesCount++;
                std::get<1>(ports).try_put(tf::continue_msg());
                return;
        }

        std::get<0>(ports).try_put(frame);
        std::get<1>(ports).try_put(tf::continue_msg());
    };

    auto frame_distributor_body = [this](const FramePtr &frame, auto &ports) {
        if (!m_cameras.contains(frame->cameraId)) {
            std::get<0>(ports).try_put(tf::continue_msg());
            return;
        }

        // TODO: Maybe check for frame expiration here as well.

        m_cameras.at(frame->cameraId).postSequencer->try_put(frame);
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

void EngineWorker::registerCamera(const QCameraDevice &cameraDevice, QVideoSink *videoSink, int maxInFlight)
{
    if (cameraDevice.isNull()) {
        qCCritical(worker_logger) << "Cannot register null camera device";
        return;
    }

    auto async_src_body = [] (const tf::continue_msg &, tf::async_node<tf::continue_msg, FramePtr>::gateway_type& gateway) {};
    auto sequencer_body = [] (const FramePtr &f) { return f->sequenceId; };

    CameraInfo cam;
    cam.asyncSrc = QSharedPointer<tf::async_node<tf::continue_msg, FramePtr>>::create(m_graph, tf::unlimited, async_src_body);
    cam.preLimiter = QSharedPointer<tf::limiter_node<FramePtr>>::create(m_graph, maxInFlight);
    cam.postSequencer = QSharedPointer<tf::sequencer_node<FramePtr>>::create(m_graph, sequencer_body);
    cam.outVideoSink = videoSink;
    cam.cameraDevice = cameraDevice;

    tf::make_edge(*cam.asyncSrc, *cam.preLimiter);
    tf::make_edge(*cam.preLimiter, *m_processor);
    tf::make_edge(tf::output_port<1>(*m_processor), cam.preLimiter->decrementer());
    tf::make_edge(*cam.postSequencer, *m_uiNotifier);

    auto thread = new QThread();
    auto worker = new CameraCapture(cameraDevice, cam.asyncSrc->gateway());
    worker->moveToThread(thread);
    connect(thread, &QThread::started, worker, &CameraCapture::init);
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    
    cam.capture = { thread, worker };
    cam.capture.thread->start();

    m_cameras.emplace(cameraDevice.id(), cam);
    emit cameraRegistered(cameraDevice);
}

// ------------------ Engine Implementation ------------------
Q_STATIC_LOGGING_CATEGORY(engine_logger, "mtgs.engine");
Engine::Engine() 
{
    auto thread = new QThread();
    auto worker = new EngineWorker(m_cameras);
    worker->moveToThread(thread);
    connect(thread, &QThread::started, worker, &EngineWorker::init);
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    connect(worker, &EngineWorker::engineLoaded, this, &Engine::setEngineLoaded);
    connect(worker, &EngineWorker::sendFrameToMainThread, this, &Engine::receiveFrameNotification);
    connect(worker, &EngineWorker::cameraRegistered, this, &Engine::cameraRegistered);

    m_engine.worker = worker;
    m_engine.thread = thread;
    thread->start();
}

Engine::~Engine() 
{
    m_engine.thread->quit();
    m_engine.thread->wait();
}

CameraModel Engine::createCameraModel() 
{
    return CameraModel(m_cameras, this);
}

QSharedPointer<CameraModel> Engine::createSharedCameraModel() 
{
    QSharedPointer<CameraModel> model = QSharedPointer<CameraModel>::create(m_cameras, this);
    connect(m_engine.worker, &EngineWorker::cameraRegistered, model.data(), &CameraModel::cameraAdded);

    return model;
}

bool Engine::isEngineLoaded() const 
{
    return m_engineLoaded;
}

void Engine::receiveFrameNotification(const FramePtr& frame) 
{
    if (!m_cameras.contains(frame->cameraId)) {
        qCDebug(engine_logger) << QString("Stranded frame %1 from camera %2").arg(frame->sequenceId).arg(frame->cameraId);
        return;
    }

    auto videoSink = m_cameras[frame->cameraId].outVideoSink;
    if (videoSink)
        videoSink->setVideoFrame(frame->originalFrame);
}

void Engine::registerCameraOutSink(const QString &cameraId, QVideoSink *videoSink) 
{
    if (!m_cameras.contains(cameraId)) {
        qCDebug(engine_logger) << QString("Trying to register unknown camera %1").arg(cameraId);
        return;
    }

    m_cameras[cameraId].outVideoSink = videoSink;
}

void Engine::registerCamera(const QCameraDevice &cameraDevice, QVideoSink *videoSink, int maxInFlight)
{
    if (m_engine.thread && m_engine.worker) {
        QMetaObject::invokeMethod(m_engine.worker, "registerCamera", Qt::AutoConnection,
            Q_ARG(QCameraDevice, cameraDevice), Q_ARG(QVideoSink*, videoSink), Q_ARG(int, maxInFlight));
    } else {
        qCCritical(engine_logger) << "Worker thread not initialized yet. Cannot register camera" << cameraDevice.description();
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