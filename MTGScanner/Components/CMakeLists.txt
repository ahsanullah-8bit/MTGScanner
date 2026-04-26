
qt_add_library(${PROJECT_NAME}Components STATIC)
qt_add_qml_module(${PROJECT_NAME}Components
    URI MTGScanner.Components
    VERSION 1.0
    QML_FILES
        SideBar.qml

    RESOURCES
        icons/configuration.svg
        icons/dashboard.svg
        icons/menu.svg
        icons/outputwindows.svg
)

# For the MTGScanner propagator to work.
target_link_libraries(${PROJECT_NAME}Components PRIVATE ${PROJECT_NAME})