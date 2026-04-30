#pragma once

#include <QRect>
#include <QList>
#include <QObject>
#include <QScreen>
#include <QCameraDevice>
#include <QLoggingCategory>
#include <QtQmlIntegration/qqmlintegration.h>

namespace MTGS {

struct ChannelOptions {
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(int maxInFlight MEMBER maxInFlight)
    Q_PROPERTY(float detectionThreshold MEMBER detectionThreshold)
    Q_PROPERTY(QList<QString> filters MEMBER filters)
    Q_PROPERTY(QCameraDevice cameraDevice MEMBER cameraDevice)
    Q_PROPERTY(QString windowName MEMBER windowName)
    Q_PROPERTY(QRect windowGeometry MEMBER windowGeometry)
    Q_PROPERTY(QString screenSerialNo MEMBER screenSerialNo)
    Q_PROPERTY(QString screenName MEMBER screenName)

    QML_VALUE_TYPE(channelOptions)
    QML_CONSTRUCTIBLE_VALUE
public:

    QString id;
    QString name;
    int maxInFlight = 16;
    float detectionThreshold = 40;
    QList<QString> filters;
    QCameraDevice cameraDevice;
    // Output
    QString windowName;
    QRect windowGeometry;
    QString screenSerialNo;
    QString screenName;

    // TODO
    Q_INVOKABLE bool isValid() const {
        return !id.isEmpty()
               && !name.isEmpty()
               && !cameraDevice.isNull()
               && !windowName.isEmpty();
    }
    bool isValid(const QLoggingCategory &logger) const { return true; }
};

} // namespace MTGS
