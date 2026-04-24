#include <QObject>
#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <engine/engine.h>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    MTGS::Engine mtgsEngine;
    qmlRegisterSingletonInstance("MTGScanner.Engine", 0, 1, "Engine", &mtgsEngine);

    QSharedPointer<MTGS::CameraModel> camerasModel = mtgsEngine.createSharedCameraModel();
    qmlRegisterSingletonInstance("MTGScanner.Engine", 0, 1, "CamerasModel", camerasModel.data());

    engine.loadFromModule("MTGScanner", "Main");

    return app.exec();
}
