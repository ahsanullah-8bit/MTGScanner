#pragma once

#include <QList>
#include <QObject>
#include <QCameraDevice>
#include <QLoggingCategory>
#include <QtQmlIntegration/qqmlintegration.h>

namespace MTGS {

struct OutputWindowOptions {
    Q_GADGET
    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(QString targetMonitor MEMBER targetMonitor)
    Q_PROPERTY(int width MEMBER width)
    Q_PROPERTY(int height MEMBER height)
    Q_PROPERTY(int x MEMBER x)
    Q_PROPERTY(int y MEMBER y)

    QML_VALUE_TYPE(outputWindowOptions)
    QML_CONSTRUCTIBLE_VALUE
public:
    QString name;
    QString targetMonitor;
    int width = 800;
    int height = 600;
    int x = 100;
    int y = 100;

    Q_INVOKABLE OutputWindowOptions() {}
    Q_INVOKABLE OutputWindowOptions(
        const QString &name, 
        const QString &targetMonitor, 
        int width, int height, 
        int x, int y)
        : name(name)
        , targetMonitor(targetMonitor)
        , width(width), height(height)
        , x(x), y(y)
        {}

    bool operator==(const OutputWindowOptions &other) const {
        return name == other.name;
    }

    bool operator!=(const OutputWindowOptions &other) const {
        return !(*this == other);
    }

    Q_INVOKABLE bool isValid() const { return !name.isEmpty()/* && !targetMonitor.isEmpty()*/; }
};

struct ChannelOptions {
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(int maxInFlight MEMBER maxInFlight)
    Q_PROPERTY(float detectionThreshold MEMBER detectionThreshold)
    Q_PROPERTY(QList<QString> filters MEMBER filters)
    Q_PROPERTY(QCameraDevice cameraDevice MEMBER cameraDevice)
    Q_PROPERTY(OutputWindowOptions outputWindow MEMBER outputWindow)

    QML_VALUE_TYPE(channelOptions)
    QML_CONSTRUCTIBLE_VALUE
public:
    Q_INVOKABLE ChannelOptions() {}
    Q_INVOKABLE ChannelOptions(
        const QString &id, const QString &name, 
        const QCameraDevice &cameraDevice, 
        int maxInFlight, const QList<QString> &filters, 
        const OutputWindowOptions &outputWindow)
        : id(id), name(name)
        , cameraDevice(cameraDevice)
        , maxInFlight(maxInFlight)
        , filters(filters)
        , outputWindow(outputWindow)
    {}

    QString id;
    QString name;
    int maxInFlight = 16;
    float detectionThreshold = 40;
    QList<QString> filters;
    QCameraDevice cameraDevice;
    OutputWindowOptions outputWindow;

    // TODO
    Q_INVOKABLE bool isValid() const {
        return !id.isEmpty()
               && !name.isEmpty()
               && !cameraDevice.isNull()
               && outputWindow.isValid();
    }
    bool isValid(const QLoggingCategory &logger) const { return true; }
};

} // namespace MTGS
