#include "channelmodel.h"
#include <qabstractitemmodel.h>
#include <qvariant.h>

namespace MTGS {

ChannelModel::ChannelModel(QObject *parent)
    : QAbstractListModel(parent)
{}

int ChannelModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return static_cast<int>(m_channels.size());
}

QVariant ChannelModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= static_cast<int>(m_channels.size()))
        return QVariant();

    const auto &options = m_channels.at(index.row());
    
    switch (role) {
        case IdRole:
            return options.id;
        case NameRole:
            return options.name;
        case DeviceRole:
            return QVariant::fromValue(options.cameraDevice);
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> ChannelModel::roleNames() const
{
    static QHash<int, QByteArray> roles {
        {IdRole, "id"},
        {NameRole, "name"},
        {DeviceRole, "device"}
    };

    return roles;
}

void ChannelModel::channelAdded(const ChannelOptions &options)
{
    int row = static_cast<int>(m_channels.size());
    beginInsertRows(QModelIndex{}, row, row);
    m_channels.append(options);
    endInsertRows();
}

void ChannelModel::channelDeleted(const ChannelOptions &options)
{
    int row = static_cast<int>(m_channels.indexOf(options));
    if (row == -1) return;

    beginRemoveRows(QModelIndex{}, row, row);
    m_channels.removeAt(row);
    endRemoveRows();
}

} // namespace MTGS