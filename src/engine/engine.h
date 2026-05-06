#pragma once

#include <tuple>

#include <tbb_patched.hpp>

#include <QObject>
#include <QString>
#include <QHash>
#include <QVideoSink>
#include <QSettings>
#include <QSharedPointer>
#include <QCameraDevice>

#include <camera/cameracapture.h>
#include <engine/channelinfo.hpp>
#include <engine/channelmodel.h>

namespace MTGS {

class EngineWorker : public QObject {
    Q_OBJECT
public:
    EngineWorker(tbb::concurrent_unordered_map<QString, QSharedPointer<ChannelInfo>> &channels, QObject *parent = nullptr);
    virtual ~EngineWorker();

public slots:
    void init();
    void saveToSettings();
    void loadFromSettings();
    void addChannel(const MTGS::ChannelOptions &channelOptions, QVideoSink *videoSink = nullptr, int channelStatus = 0, QScreen *screen = nullptr);
    void deleteChannel(const MTGS::ChannelOptions &options);
    void startChannel(const QString &channelId);
    void stopChannel(const QString &channelId);

signals:
    void engineLoaded(bool loaded = true);
    void channelAdded(const MTGS::ChannelOptions &channelOptions);
    void channelDeleted(const MTGS::ChannelOptions &options);
    void channelStarted(bool started = true);
    void channelStopped(bool stopped = true);
    void sendFrameToMainThread(const MTGS::FramePtr& frame);

private:
    tf::graph m_graph;
    QSharedPointer<tf::multifunction_node<FramePtr, std::tuple<FramePtr, tf::continue_msg>>> m_processor;
    QSharedPointer<tf::multifunction_node<FramePtr, std::tuple<tf::continue_msg>>> m_frameDistributor;
    QSharedPointer<tf::function_node<FramePtr>> m_uiNotifier;

    tbb::concurrent_unordered_map<QString, QSharedPointer<ChannelInfo>> &m_channels;
};

class Engine : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool engineLoaded READ isEngineLoaded WRITE setEngineLoaded NOTIFY engineLoaded FINAL)
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
    Q_INVOKABLE QString createChannelId() const;
    Q_INVOKABLE ChannelOptions createChannelOptions() const;
    Q_INVOKABLE ChannelOptions channelOptions(const QString &channelId) const;
    Q_INVOKABLE QObject *channelMetrics(const QString &channelId) const;

public slots:
    void receiveFrameNotification(const MTGS::FramePtr& frame);
    void addChannel(const ChannelOptions &channelOptions, QVideoSink *videoSink = nullptr, int channelStatus = 0, QScreen *screen = nullptr);
    void deleteChannel(const MTGS::ChannelOptions &options);
    void startChannel(const QString &channelId);
    void stopChannel(const QString &channelId);
    void registerChannelOutSink(const QString &channelId, QVideoSink *videoSink);

signals:
    void engineLoaded(bool loaded = true);
    void channelAdded(const MTGS::ChannelOptions &channelOptions);
    void channelDeleted(const MTGS::ChannelOptions &options);
    void channelStarted(bool started = true);
    void channelStopped(bool stopped = true);

private:
    void setEngineLoaded(bool loaded);

private:
    bool m_engineLoaded = false;
    struct {
        EngineWorker *worker;
        QThread *thread;
    } m_engine;
    tbb::concurrent_unordered_map<QString, QSharedPointer<ChannelInfo>> m_channels;
};

} // namespace MTGS