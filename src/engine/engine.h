#pragma once

#include <qobject.h>
#include <qtmetamacros.h>
#include <tuple>

#include <tbb_patched.hpp>

#include <QList>
#include <QHash>
#include <QTimer>
#include <QObject>
#include <QString>
#include <QVideoSink>
#include <QSettings>
#include <QSharedPointer>
#include <QCameraDevice>

#include <camera/availablecamerasmodel.h>
#include <camera/cameracapture.h>
#include <camera/cameramanager.h>
#include <engine/channel.hpp>
#include <engine/channelmodel.h>

namespace MTGS {

class EngineWorker : public QObject {
    Q_OBJECT
public:
    EngineWorker(QObject *parent = nullptr);
    virtual ~EngineWorker();

public slots:
    void init();
    void addChannel(QSharedPointer<ChannelRaw> channel);
    void deleteChannel(const MTGS::ChannelOptions &options);
    void onChannelErrorOccurred(const QString &channelId, QCamera::Error error, const QString &errorStr);
    void onChannelActiveChanged(const QString &channelId, bool active);

signals:
    void engineLoaded(bool loaded = true);
    void channelReady(const MTGS::ChannelOptions &options);
    void channelDeleted(const MTGS::ChannelOptions &options);
    void sendFrameToMainThread(const MTGS::FramePtr& frame);

private:
    tf::graph m_graph;
    QSharedPointer<tf::multifunction_node<FramePtr, std::tuple<FramePtr, tf::continue_msg>>> m_processor;
    QSharedPointer<tf::multifunction_node<FramePtr, std::tuple<tf::continue_msg>>> m_frameDistributor;
    QSharedPointer<tf::function_node<FramePtr>> m_uiNotifier;

    tbb::concurrent_hash_map<QString, QSharedPointer<ChannelRaw>> m_channels;
};

// Engine
class Engine : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool engineLoaded READ isEngineLoaded WRITE setEngineLoaded NOTIFY engineLoaded FINAL)
    Q_PROPERTY(MTGS::ChannelModel* channelsModel READ channelsModel CONSTANT)
    Q_PROPERTY(MTGS::AvailableCamerasModel* availableCamerasModel READ availableCamerasModel CONSTANT)
public:
    enum ChannelStatus {
        Unknown,
        Initializing,
        Starting,
        Running,
        Stopping,
        Stopped,
        Errored
    };
    Q_ENUM(ChannelStatus)

    Engine(QObject *parent = nullptr);
    ~Engine();
    bool isEngineLoaded() const;
    ChannelModel* channelsModel() const;
    AvailableCamerasModel* availableCamerasModel() const;
    Q_INVOKABLE QObject *createChannel();
    Q_INVOKABLE void destroyChannel(QObject *channel);
    Q_INVOKABLE QObject *channel(const QString &channelId);
    Q_INVOKABLE QObject *channelAtIndex(int index);
    Q_INVOKABLE bool channelExists(const QString &id);
    CameraManager *cameraManager();

public slots:
    void saveToSettings();
    void loadFromSettings();
    void receiveFrameNotification(const MTGS::FramePtr& frame);
    void addChannel(QObject *channelObj, int status = ChannelStatus::Unknown, QScreen *screen = nullptr);
    void deleteChannel(const MTGS::ChannelOptions &options);
    void startChannel(const QString &channelId);
    void stopChannel(const QString &channelId);
    void registerChannelOutSink(const QString &channelId, QVideoSink *videoSink);
    void unRegisterChannelOutSink(const QString &channelId);

signals:
    void engineLoaded(bool loaded = true);
    void channelAdded(const MTGS::ChannelOptions &options);
    void channelAboutToBeDeleted(const MTGS::ChannelOptions &options);
    void channelDeleted(const MTGS::ChannelOptions &options);
    void channelStarted(bool started = true);
    void channelStopped(bool stopped = true);

private slots:
    void onChannelReady(const MTGS::ChannelOptions &options);
    void onChannelDeleted(const MTGS::ChannelOptions &options);

private:
    void setEngineLoaded(bool loaded);

private:
    bool m_engineLoaded = false;
    struct {
        EngineWorker *worker;
        QThread *thread;
    } m_engine;

    AvailableCamerasModel *m_availableCamerasModel = nullptr;
    ChannelModel *m_channelsModel = nullptr;
    CameraManager *m_cameraMngr = nullptr;
    QHash<QString, Channel*> m_channels;

    QTimer *m_metricsTimer = nullptr;
    QObject *ch;
};

} // namespace MTGS