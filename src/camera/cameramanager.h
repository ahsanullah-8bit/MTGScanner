#pragma once

#include <QList>
#include <QHash>
#include <QObject>
#include <QMediaDevices>
#include <QCameraDevice>
#include <optional>
#include <qcameradevice.h>
#include <qcontainerfwd.h>

namespace MTGS {
    
class CameraManager : public QObject {
    Q_OBJECT
public:
    explicit CameraManager(QObject *parent = nullptr);
    QList<QCameraDevice> availableCameras() const;
    bool isCameraInUse(const QCameraDevice& device) const;
    std::optional<QCameraDevice> findBy(const QString &, const QString &) const;
    
public slots:
    void setCameraInUse(const QCameraDevice &camera, bool inUse);
    void onVideoInputsChanged();

signals:
    void availableCamerasChanged();
    void cameraStatusChanged(const QCameraDevice&, bool);
    void cameraAdded(const QCameraDevice&);
    void cameraRemoved(const QCameraDevice&);
    void cameraModified(const QCameraDevice&);

private:
    QMediaDevices *m_mediaDevices = nullptr;
    QHash<QString, bool> m_inUse;
    QList<QCameraDevice> m_cameras;
};

}