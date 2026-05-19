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

#include <camera/availablecamerasmodel.h>
#include <camera/cameracapture.h>
#include <camera/cameramanager.h>
#include <engine/channelmodel.h>
#include <engine/channelraw.hpp>
#include <channel.hpp>

namespace MTGS {

// Engine
class Engine : public QObject {
    Q_OBJECT
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
    ChannelModel* channelsModel() const;
    AvailableCamerasModel* availableCamerasModel() const;
    Q_INVOKABLE Channel *createChannel();
    Q_INVOKABLE Channel *channel(const QString &channelId);
    Q_INVOKABLE Channel *channelAtIndex(int index);
    Q_INVOKABLE void destroyChannel(Channel *channel);
    Q_INVOKABLE bool channelExists(const QString &id);
    CameraManager *cameraManager();

public slots:
    void saveToSettings();
    void loadFromSettings();
    void receiveFrameNotification(const MTGS::FramePtr& frame);
    void addChannel(Channel *channel, int status = ChannelStatus::Unknown, QScreen *screen = nullptr);
    void deleteChannel(const MTGS::ChannelOptions &options);
    void startChannel(const QString &channelId);
    void stopChannel(const QString &channelId);
    void registerChannelOutSink(const QString &channelId, QVideoSink *videoSink);
    void unRegisterChannelOutSink(const QString &channelId);

signals:
    void channelAdded(const MTGS::ChannelOptions &options);
    void channelAboutToBeDeleted(const MTGS::ChannelOptions &options);
    void channelDeleted(const MTGS::ChannelOptions &options);
    void channelStarted(bool started = true);
    void channelStopped(bool stopped = true);

private slots:
    void initializeGraph();

private:
    void disconnectChannel(Channel *channel, QSharedPointer<ChannelRaw> raw_channel = nullptr);

private:
    tf::graph m_graph;
    QSharedPointer<tf::multifunction_node<FramePtr, std::tuple<FramePtr, tf::continue_msg>>> m_processor;
    QSharedPointer<tf::multifunction_node<FramePtr, std::tuple<tf::continue_msg>>> m_frameDistributor;
    QSharedPointer<tf::function_node<FramePtr>> m_uiNotifier;

    tbb::concurrent_hash_map<QString, QSharedPointer<ChannelRaw>> m_rawChannels;
    QHash<QString, Channel*> m_channels;
    QList<QString> m_channelIdIndexMap;

    AvailableCamerasModel *m_availableCamerasModel = nullptr;
    CameraManager *m_cameraMngr = nullptr;
    ChannelModel *m_channelsModel = nullptr;
    
    QTimer *m_metricsTimer = nullptr;
};

} // namespace MTGS