#include "cameramanager.h"
#include <optional>

namespace MTGS {

CameraManager::CameraManager(QObject *parent)
    : QObject(parent)
    , m_mediaDevices(new QMediaDevices(this))
{
    connect(m_mediaDevices, &QMediaDevices::videoInputsChanged, this, &CameraManager::onVideoInputsChanged);

    onVideoInputsChanged();
}

QList<QCameraDevice> CameraManager::availableCameras() const
{
    QList<QCameraDevice> availables;
    for (const auto &cam : m_cameras) {
        if (!m_inUse.value(cam.id(), false))
            availables.append(cam);
    }

    return availables;
}

bool CameraManager::isCameraInUse(const QCameraDevice& device) const
{
    return m_inUse.value(device.id(), false);
}

std::optional<QCameraDevice> CameraManager::findBy(const QString &id, const QString &description) const
{
    const auto cam_it = std::find_if(m_cameras.begin(), m_cameras.end(),
        [&id, &description] (const QCameraDevice &camera) {
            return camera.id() == id || camera.description() == description;
        }
    );

    return cam_it != m_cameras.end() ? std::make_optional(*cam_it) : std::nullopt;
}

void CameraManager::setCameraInUse(const QCameraDevice &camera, bool inUse)
{
    m_inUse[camera.id()] = inUse;
    emit cameraStatusChanged(camera, inUse);
    emit availableCamerasChanged();
}

void CameraManager::onVideoInputsChanged()
{
    const auto currentInputs = QMediaDevices::videoInputs();

    // Remove
    for (qsizetype i = m_cameras.size() - 1; i >= 0; --i) {
        const auto &existing = m_cameras.at(i);
        if (currentInputs.contains(existing))
            continue;

        m_inUse.remove(existing.id());
        emit cameraRemoved(existing);
        m_cameras.removeAt(i);
    }

    // Add
    for (const auto &device : currentInputs) {
        if (m_cameras.contains(device))
            continue;

        m_cameras.append(device);
        m_inUse[device.id()] = false;
        emit cameraAdded(device);
    }

    emit availableCamerasChanged();
}

}