
qt_add_library(${PROJECT_NAME}Components STATIC)
qt_add_qml_module(${PROJECT_NAME}Components
    URI MTGScanner.Components
    VERSION 1.0
    QML_FILES
        ActiveFiltersCard.qml
        CameraPreviewCard.qml
        CameraSourceCard.qml
        HeaderSection.qml
        OutputWindowCard.qml
        ChannelWizard.qml
        SideBar.qml
        StatusSection.qml
)

# For the MTGScanner propagator to work.
target_link_libraries(${PROJECT_NAME}Components PRIVATE ${PROJECT_NAME})