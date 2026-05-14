#pragma once

#include <qabstractitemmodel.h>
#include <qcameradevice.h>
#include <qlist.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qtmetamacros.h>

#include <camera/cameramanager.h>

namespace MTGS {

class AvailableCamerasModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum CameraDeviceRoles {
        Id = Qt::UserRole + 1,
        Description,
        IsDefault,
        IsNull,
        PhotoResolutions,
        Position,
        VideoFormats
    };

    explicit AvailableCamerasModel(QObject *parent = nullptr);
    void setCameraManager(CameraManager *manager);

    // QAbstractItemModel interface
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

public slots:
    void onAvailableCamerasChanged();

private:
    QList<QCameraDevice> m_availableCameras;
    CameraManager *m_mngr = nullptr;
};

}