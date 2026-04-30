#include <QObject>
#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <engine/engine.h>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QCoreApplication::setOrganizationName("MadStudio");
    QCoreApplication::setOrganizationDomain("MadStudio.io");
    QCoreApplication::setApplicationName("MTGScanner");

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    MTGS::Engine mtgsEngine;
    qmlRegisterSingletonInstance("MTGScanner.Engine", 0, 1, "Engine", &mtgsEngine);

    QSharedPointer<MTGS::ChannelModel> channelsModel = mtgsEngine.createSharedChannelModel();
    qmlRegisterSingletonInstance("MTGScanner.Engine", 0, 1, "ChannelsModel", channelsModel.data());

    engine.loadFromModule("MTGScanner", "Main");

    return app.exec();
}
