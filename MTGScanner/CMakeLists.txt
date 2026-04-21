qt_add_library(${PROJECT_NAME} STATIC)
qt_add_qml_module(${PROJECT_NAME}
    URI MTGScanner
    VERSION 1.0
    QML_FILES
        Main.qml
)