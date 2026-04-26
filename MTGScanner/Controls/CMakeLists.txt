
qt_add_library(${PROJECT_NAME}Controls STATIC)
qt_add_qml_module(${PROJECT_NAME}Controls
    URI MTGScanner.Controls
    VERSION 1.0
    QML_FILES
        IconLabel.qml
        TabButton.qml
)

# For the MTGScanner propagator to work.
target_link_libraries(${PROJECT_NAME}Controls PRIVATE ${PROJECT_NAME})