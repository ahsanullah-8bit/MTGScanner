#include <QObject>
#include <QQmlEngine>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QScopedPointer>

#include <engine/engine.h>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QCoreApplication::setOrganizationName("MadStudio");
    QCoreApplication::setOrganizationDomain("MadStudio.io");
    QCoreApplication::setApplicationName("MTGScanner");

    QScopedPointer<MTGS::Engine> mtgs_engine(new MTGS::Engine);
    qmlRegisterSingletonInstance("MTGScanner.Engine", 0, 1, "Engine", mtgs_engine.get());

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("MTGScanner", "Main");

    return app.exec();
}
