#include "cameramodel.h"

namespace MTGS {

CameraModel::CameraModel(tbb::concurrent_unordered_map<QString, CameraInfo> &cameras, QObject *parent)
    : QAbstractListModel(parent), m_cameras(cameras) 
{}

int CameraModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return static_cast<int>(m_cameras.size());
}

QVariant CameraModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= static_cast<int>(m_cameras.size()))
        return QVariant();

    auto it = m_cameras.begin();
    std::advance(it, index.row());
    const auto &camInfo = it->second;
    
    switch (role) {
        case IdRole:
            return it->first;
        case DescriptionRole:
            return camInfo.cameraDevice.description();
        case VideoSinkRole:
            return QVariant::fromValue(camInfo.outVideoSink);
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> CameraModel::roleNames() const
{
    static QHash<int, QByteArray> roles {
        {IdRole, "cameraId"},
        {DescriptionRole, "description"},
        {VideoSinkRole, "videoSink"}
    };

    return roles;
}

} // namespace MTGS