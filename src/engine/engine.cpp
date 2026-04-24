
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
    auto cameras = QMediaDevices::videoInputs();

    // Initialize camera info, workers and their nodes.
    for (const auto &camera : cameras) {
        qCDebug(worker_logger) << QString("Initializing camera %1").arg(camera.id());

        CameraInfo info;
        auto async_src_body = [] (const tf::continue_msg &, tf::async_node<tf::continue_msg, FramePtr>::gateway_type& gateway) {};
        info.asyncSrc = QSharedPointer<tf::async_node<tf::continue_msg, FramePtr>>::create(m_graph, tf::unlimited, async_src_body);
        info.sequencer = QSharedPointer<tf::sequencer_node<FramePtr>>::create(m_graph, [](const FramePtr &f) {
            return f->sequenceId;
        });
        info.limiter = QSharedPointer<tf::limiter_node<FramePtr>>::create(m_graph, 30);
        tf::make_edge(*info.asyncSrc, *info.limiter);
        // This is initialized in the main thread (Engine).
        info.outVideoSink = nullptr;
        info.cameraDevice = camera;

        auto thread = new QThread();
        auto worker = new CameraCapture(camera, info.asyncSrc->gateway());
        worker->moveToThread(thread);
        connect(thread, &QThread::started, worker, &CameraCapture::init);
        connect(thread, &QThread::finished, worker, &QObject::deleteLater);
        connect(thread, &QThread::finished, thread, &QObject::deleteLater);

        info.capture = {thread, worker};
        m_cameras.emplace(camera.id(), info);
    }

    // The critical processor node.
    m_processor = QSharedPointer<tf::multifunction_node<FramePtr, std::tuple<FramePtr, tf::continue_msg>>>::create(m_graph, tf::unlimited, 
        [this](const FramePtr &frame, auto &ports) {
            if (frame->timestamp.msecsTo(QTime::currentTime()) > 100) {
                qCWarning(worker_logger) << QString("Cam %1 frame at %2 expired")
                    .arg(m_cameras.at(frame->cameraId).cameraDevice.description())
                    .arg(frame->originalFrame.startTime());

                    std::get<1>(ports).try_put(tf::continue_msg());
                    return;
            }

            // Process the image. For now, we just pass it through.

            std::get<0>(ports).try_put(frame);
            std::get<1>(ports).try_put(tf::continue_msg());
        }
    );

    m_frameDistributor = QSharedPointer<tf::multifunction_node<FramePtr, std::tuple<tf::continue_msg>>>::create(m_graph, tf::unlimited, 
        [this](const FramePtr &frame, auto &ports) {
            if (frame->timestamp.msecsTo(QTime::currentTime()) > 100) {
                qCWarning(worker_logger) << QString("Cam %1 frame at %2 expired at distributor")
                    .arg(m_cameras.at(frame->cameraId).cameraDevice.description())
                    .arg(frame->originalFrame.startTime());

                    std::get<0>(ports).try_put(tf::continue_msg());
                    return;
            }

            if (!m_cameras.contains(frame->cameraId)) {
                std::get<0>(ports).try_put(tf::continue_msg());
                return;
            }

            // Assigning and increasing the sequence count
            frame->sequenceId = m_cameras.at(frame->cameraId).sequenceCount++;

            m_cameras.at(frame->cameraId).sequencer->try_put(frame);
            std::get<0>(ports).try_put(tf::continue_msg());
        }
    );

    // Final frame notifier.
    auto output_notifier_body = [this](const FramePtr &f) {
        f->originalFrame = QVideoFrame(QImage(f->mat.data, f->mat.cols, f->mat.rows, f->mat.step, QImage::Format_BGR888).copy());
        
        QMetaObject::invokeMethod(this, "sendFrameToMainThread", Qt::AutoConnection, Q_ARG(FramePtr, f));
    };
    m_outputNotifier = QSharedPointer<tf::function_node<FramePtr>>::create(m_graph, tf::serial, output_notifier_body);

    // Connect camera nodes to processor.
    for (const auto &[_, cam] : m_cameras) {
        tf::make_edge(*cam.limiter, *m_processor);
        tf::make_edge(tf::output_port<1>(*m_processor), cam.limiter->decrementer());
        tf::make_edge(*cam.sequencer, *m_outputNotifier);
    }
    
    // Now that everything is set up, start the camera threads.
    for (const auto &[_, cam] : m_cameras) {
        cam.capture.thread->start();
    }

    emit engineLoaded();
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

    m_workerThread = qMakePair(thread, worker);
    thread->start();
}

Engine::~Engine() 
{
    m_workerThread.first->quit();
    m_workerThread.first->wait();
}

CameraModel Engine::createCameraModel() 
{
    return CameraModel(m_cameras, this);
}

QSharedPointer<CameraModel> Engine::createSharedCameraModel() 
{
    QSharedPointer<CameraModel> model = QSharedPointer<CameraModel>::create(m_cameras, this);
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

void Engine::setEngineLoaded(bool loaded) 
{
    if (m_engineLoaded == loaded)
        return;

    m_engineLoaded = loaded;
    emit engineLoaded(loaded);
}

} // namespace MTGS