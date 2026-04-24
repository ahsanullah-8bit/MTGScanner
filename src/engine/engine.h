#pragma once

#include <tuple>

#include <tbb_patched.hpp>

#include <QObject>
#include <QString>
#include <QHash>
#include <QVideoSink>
#include <QCameraDevice>

#include <camera/cameracapture.h>
#include <engine/camerainfo.hpp>
#include <engine/cameramodel.h>

namespace MTGS {

class EngineWorker : public QObject {
    Q_OBJECT
public:
    EngineWorker(tbb::concurrent_unordered_map<QString, CameraInfo> &cameras, QObject *parent = nullptr);
    virtual ~EngineWorker();

public slots:
    void init();
    
signals:
    void engineLoaded(bool loaded = true);
    void sendFrameToMainThread(const FramePtr& frame);

private:
    tf::graph m_graph;
    QSharedPointer<tf::multifunction_node<FramePtr, std::tuple<FramePtr, tf::continue_msg>>> m_processor;
    QSharedPointer<tf::multifunction_node<FramePtr, std::tuple<tf::continue_msg>>> m_frameDistributor;
    QSharedPointer<tf::function_node<FramePtr>> m_outputNotifier;

    tbb::concurrent_unordered_map<QString, CameraInfo> &m_cameras;
};

class Engine : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool engineLoaded READ isEngineLoaded WRITE setEngineLoaded NOTIFY engineLoaded FINAL)
public:
    Engine();
    ~Engine();
    CameraModel createCameraModel();
    QSharedPointer<CameraModel> createSharedCameraModel();
    bool isEngineLoaded() const;

public slots:
    void receiveFrameNotification(const FramePtr& frame);
    Q_INVOKABLE void registerCameraOutSink(const QString &cameraId, QVideoSink *videoSink);

signals:
    void engineLoaded(bool loaded = true);

private:
    void setEngineLoaded(bool loaded);

private:
    bool m_engineLoaded = false;
    QPair<QThread*, EngineWorker*> m_workerThread;
    tbb::concurrent_unordered_map<QString, CameraInfo> m_cameras;
};

} // namespace MTGS