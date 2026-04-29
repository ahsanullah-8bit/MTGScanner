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
        ChannelStatus,
        CameraDeviceRole,
    };

    explicit ChannelModel(tbb::concurrent_unordered_map<QString, ChannelInfo> &channel, QObject *parent = nullptr);

    // QAbstractItemModel interface
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    // bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QHash<int, QByteArray> roleNames() const override;

public slots:
    void channelAdded(const ChannelOptions &cameraDevice);
    void channelRemoved(const ChannelOptions &cameraDevice);

private:
    tbb::concurrent_unordered_map<QString, ChannelInfo> &m_channel;
};

} // namespace MTGS