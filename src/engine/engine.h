#pragma once

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

#include <camera/cameracapture.h>
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

class Engine : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool engineLoaded READ isEngineLoaded WRITE setEngineLoaded NOTIFY engineLoaded FINAL)
    Q_PROPERTY(QObject* currentChannel READ currentChannel WRITE setCurrentChannel NOTIFY currentChannelChanged FINAL)
    Q_PROPERTY(QList<QCameraDevice> availableCameras READ availableCameras NOTIFY availableCamerasChanged FINAL)
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
    QSharedPointer<ChannelModel> createSharedChannelModel();
    bool isEngineLoaded() const;
    Q_INVOKABLE QObject *createChannel();
    Q_INVOKABLE void destroyChannel(QObject *channel);
    Q_INVOKABLE QObject *channel(const QString &channelId);
    QObject *currentChannel();
    QList<QCameraDevice> availableCameras() const;

public slots:
    void saveToSettings();
    void loadFromSettings();
    void receiveFrameNotification(const MTGS::FramePtr& frame);
    void addChannel(QObject *channelObj, int status = ChannelStatus::Unknown, QScreen *screen = nullptr);
    void deleteChannel(const MTGS::ChannelOptions &options);
    void startChannel(const QString &channelId);
    void stopChannel(const QString &channelId);
    void setCurrentChannel(QObject *channel);
    void setCurrentChannelById(const QString &channelId);
    void registerChannelOutSink(const QString &channelId, QVideoSink *videoSink);
    void unRegisterChannelOutSink(const QString &channelId);

signals:
    void engineLoaded(bool loaded = true);
    void channelAdded(const MTGS::ChannelOptions &channelOptions);
    void channelDeleted(const MTGS::ChannelOptions &options);
    void channelStarted(bool started = true);
    void channelStopped(bool stopped = true);
    void currentChannelChanged();
    void availableCamerasChanged();

private:
    void setEngineLoaded(bool loaded);

private:
    bool m_engineLoaded = false;
    struct {
        EngineWorker *worker;
        QThread *thread;
    } m_engine;

    Channel *m_currentChannel = nullptr;
    QHash<QString, Channel*> m_channels;
    QList<QCameraDevice> m_availableCameras;

    QTimer *m_metricsTimer = nullptr;
};

} // namespace MTGS