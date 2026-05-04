#include "channelmodel.h"

namespace MTGS {

ChannelModel::ChannelModel(tbb::concurrent_unordered_map<QString, QSharedPointer<ChannelInfo>> &channel, QObject *parent)
    : QAbstractListModel(parent), m_channel(channel) 
{}

int ChannelModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return static_cast<int>(m_channel.size());
}

QVariant ChannelModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= static_cast<int>(m_channel.size()))
        return QVariant();

    auto it = m_channel.begin();
    std::advance(it, index.row());
    const auto &channelInfo = it->second;
    
    switch (role) {
        case ChannelIdRole:
            return it->first;
        case ChannelNameRole:
            return channelInfo->channelOptions.name;
        case ChannelOnlineStatus:
            return channelInfo->isValid;
        case CameraDeviceRole:
            return QVariant::fromValue(channelInfo->channelOptions.cameraDevice);
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> ChannelModel::roleNames() const
{
    static QHash<int, QByteArray> roles {
        {ChannelIdRole, "channelId"},
        {ChannelNameRole, "channelName"},
        {ChannelOnlineStatus, "online"},
        {CameraDeviceRole, "device"}
    };

    return roles;
}

void ChannelModel::channelAdded(const ChannelOptions &options)
{
    Q_UNUSED(options)
    beginResetModel();
    // ...
    endResetModel();
}

void ChannelModel::channelDeleted(const ChannelOptions &options)
{
    Q_UNUSED(options)
    beginResetModel();
    // ...
    endResetModel();
}

} // namespace MTGS