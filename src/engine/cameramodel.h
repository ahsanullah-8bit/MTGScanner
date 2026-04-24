#pragma once

#include <tbb_patched.hpp>

#include <QAbstractListModel>

#include <engine/camerainfo.hpp>

namespace MTGS {

class CameraModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum CameraRoles {
        IdRole = Qt::UserRole + 1,
        DescriptionRole,
        VideoSinkRole
    };

    explicit CameraModel(tbb::concurrent_unordered_map<QString, CameraInfo> &cameras, QObject *parent = nullptr);

    // QAbstractItemModel interface
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    // bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QHash<int, QByteArray> roleNames() const override;

private:
    tbb::concurrent_unordered_map<QString, CameraInfo> &m_cameras;
};

} // namespace MTGS