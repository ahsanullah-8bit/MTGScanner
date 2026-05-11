#pragma once

#include <tbb_patched.hpp>

#include <QHash>
#include <QCameraDevice>
#include <QAbstractListModel>

#include <engine/channel.hpp>

namespace MTGS {

class ChannelModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum CameraRoles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        StatusRole,
        DeviceRole,
    };

    explicit ChannelModel(QHash<QString, Channel*> &channel, QObject *parent = nullptr);

    // QAbstractItemModel interface
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    // bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QHash<int, QByteArray> roleNames() const override;

public slots:
    void channelAdded(const ChannelOptions &options);
    void channelDeleted(const ChannelOptions &options);

private:
    QHash<QString, Channel*> &m_channel;
};

} // namespace MTGS