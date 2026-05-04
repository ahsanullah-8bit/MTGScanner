#pragma once

#include <tbb_patched.hpp>

#include <QCameraDevice>
#include <QAbstractListModel>

#include <engine/channelinfo.hpp>

namespace MTGS {

class ChannelModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum CameraRoles {
        ChannelIdRole = Qt::UserRole + 1,
        ChannelNameRole,
        ChannelOnlineStatus,
        CameraDeviceRole,
    };

    explicit ChannelModel(tbb::concurrent_unordered_map<QString, QSharedPointer<ChannelInfo>> &channel, QObject *parent = nullptr);

    // QAbstractItemModel interface
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    // bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QHash<int, QByteArray> roleNames() const override;

public slots:
    void channelAdded(const ChannelOptions &options);
    void channelDeleted(const ChannelOptions &options);

private:
    tbb::concurrent_unordered_map<QString, QSharedPointer<ChannelInfo>> &m_channel;
};

} // namespace MTGS