cmake_minimum_required(VERSION 3.16)

project(MTGScanner VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.8 REQUIRED COMPONENTS Quick Multimedia QuickControls2)
find_package(OpenCV CONFIG REQUIRED)
find_package(TBB CONFIG REQUIRED)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(${PROJECT_NAME}App
    src/main.cpp

    src/camera/cameracapture.h
    src/camera/cameracapture.cpp
    src/core/frame.hpp
    src/core/prediction.hpp
    src/engine/channel.hpp
    src/engine/channelmetrics.h
    src/engine/channelmetrics.cpp
    src/engine/channelmodel.h
    src/engine/channelmodel.cpp
    src/engine/engine.h
    src/engine/engine.cpp
    src/engine/framespersecond.hpp
)

qt_add_resources(${CMAKE_PROJECT_NAME}App "configuration"
    PREFIX "/"
    FILES
        qtquickcontrols2.conf
)

add_subdirectory(MTGScanner)

set_target_properties(${PROJECT_NAME}App PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_include_directories(${PROJECT_NAME}App
    PRIVATE

    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}App
    PRIVATE

    Qt6::Quick
    Qt6::Multimedia

    TBB::tbb
    ${OpenCV_LIBS}

    MTGScanner  # the static library, just for channeloptions.hpp
    MTGScannerplugin
    MTGScannerComponentsplugin
    MTGScannerControlsplugin
)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}App
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Copy the compile_commands.json for tools like clangd
if (CMAKE_EXPORT_COMPILE_COMMANDS)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        "${CMAKE_BINARY_DIR}/compile_commands.json" 
        "${CMAKE_SOURCE_DIR}/compile_commands.json"
    )
endif()