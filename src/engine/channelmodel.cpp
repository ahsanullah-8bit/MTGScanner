#include "channelmodel.h"

namespace MTGS {

ChannelModel::ChannelModel(tbb::concurrent_hash_map<QString, QSharedPointer<ChannelInfo>> &channel, QObject *parent)
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

    auto it = m_channel.cbegin();
    std::advance(it, index.row());
    const auto &channelInfo = it->second;
    
    switch (role) {
        case IdRole:
            return it->first;
        case NameRole:
            return channelInfo->channelOptions.name;
        case StatusRole:
            return channelInfo->metrics->status();
        case DeviceRole:
            return QVariant::fromValue(channelInfo->channelOptions.cameraDevice);
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> ChannelModel::roleNames() const
{
    static QHash<int, QByteArray> roles {
        {IdRole, "id"},
        {NameRole, "name"},
        {StatusRole, "status"},
        {DeviceRole, "device"}
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