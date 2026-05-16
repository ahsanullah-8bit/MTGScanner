#pragma once

#include <qtmetamacros.h>
#include <tbb_patched.hpp>

#include <QHash>
#include <QCameraDevice>
#include <QAbstractListModel>

#include <channel.hpp>

namespace MTGS {

class ChannelModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum CameraRoles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        DeviceRole,
    };

    explicit ChannelModel(QObject *parent = nullptr);

    // QAbstractItemModel interface
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    // bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QHash<int, QByteArray> roleNames() const override;

public slots:
    void channelAdded(const ChannelOptions &options);
    void channelDeleted(const ChannelOptions &options);

private:
    QList<ChannelOptions> m_channels;
};

} // namespace MTGS