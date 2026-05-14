#include "availablecamerasmodel.h"
#include <qabstractitemmodel.h>
#include <qcameradevice.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qtpreprocessorsupport.h>
#include <qvariant.h>


namespace MTGS {

AvailableCamerasModel::AvailableCamerasModel(QObject *parent)
    : QAbstractListModel(parent)
{}

void AvailableCamerasModel::setCameraManager(CameraManager *manager)
{
    if (!manager)
        return;

    m_mngr = manager;
    connect(m_mngr, &CameraManager::availableCamerasChanged, this, &AvailableCamerasModel::onAvailableCamerasChanged);
    onAvailableCamerasChanged();
}

int AvailableCamerasModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_availableCameras.size();
}

QVariant AvailableCamerasModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= static_cast<int>(m_availableCameras.size()))
        return QVariant{};

    const QCameraDevice &device = m_availableCameras.at(index.row());

    switch (role) {
        case Id: return device.id();
        case Description: return device.description();
        case IsDefault: return device.isDefault();
        case IsNull: return device.isNull();
        case PhotoResolutions: return QVariant::fromValue(device.photoResolutions());
        case Position: return device.position();
        case VideoFormats: return QVariant::fromValue(device.videoFormats());
    }

    return QVariant{};
}

QHash<int, QByteArray> AvailableCamerasModel::roleNames() const
{
    static QHash<int, QByteArray> roles = {
        {Id, "id"},
        {Description, "description"},
        {IsDefault, "isdefault"},
        {IsNull, "isnull"},
        {PhotoResolutions, "photores"},
        {Position, "pos"},
        {VideoFormats, "vidformats"}
    };

    return roles;
}

void AvailableCamerasModel::onAvailableCamerasChanged()
{
    if (!m_mngr) return;

    beginResetModel();
    m_availableCameras = m_mngr->availableCameras();
    endResetModel();
}

}