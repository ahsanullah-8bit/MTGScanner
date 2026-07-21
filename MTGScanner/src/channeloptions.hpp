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

    QML_VALUE_TYPE(channelOptions)
    QML_CONSTRUCTIBLE_VALUE

public:
    QString id;
    QString name;
    int maxInFlight = 2;
    float detectionThreshold = 40;
    QList<QString> filters;
    QCameraDevice cameraDevice;
    // Output
    QString windowName;
    QRect windowGeometry;

    // TODO
    Q_INVOKABLE bool isValid() const {
        return !id.isEmpty()
               && !name.isEmpty()
               && !cameraDevice.isNull()
               && !windowName.isEmpty();
    }

    friend bool operator==(const ChannelOptions& lhs, const ChannelOptions& rhs) {
        return lhs.id == rhs.id &&
               lhs.name == rhs.name &&
               lhs.maxInFlight == rhs.maxInFlight &&
               qFuzzyCompare(lhs.detectionThreshold, rhs.detectionThreshold) &&
               lhs.filters == rhs.filters &&
               lhs.cameraDevice == rhs.cameraDevice &&
               lhs.windowName == rhs.windowName &&
               lhs.windowGeometry == rhs.windowGeometry;
    }

    friend bool operator!=(const ChannelOptions& lhs, const ChannelOptions& rhs) {
        return !(lhs == rhs);
    }
};

} // namespace MTGS
