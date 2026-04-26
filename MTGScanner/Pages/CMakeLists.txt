
qt_add_library(${PROJECT_NAME}Pages STATIC)
qt_add_qml_module(${PROJECT_NAME}Pages
    URI MTGScanner.Pages
    VERSION 1.0
    QML_FILES
        Dashboard.qml
        CameraConfiguration.qml
        OutputWindows.qml
)

# For the MTGScanner propagator to work.
target_link_libraries(${PROJECT_NAME}Pages PRIVATE ${PROJECT_NAME})