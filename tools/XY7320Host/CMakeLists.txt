cmake_minimum_required(VERSION 3.20)

project(XY7320Host VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick SerialPort)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(XY7320Host
    main.cpp
    backend/FirmwareUploader.cpp
    backend/FirmwareUploader.h
    backend/SerialDebug.cpp
    backend/SerialDebug.h
)

file(GLOB QML_COMPONENTS CONFIGURE_DEPENDS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "components/*.qml")
file(GLOB QML_PAGES CONFIGURE_DEPENDS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "pages/*.qml")

qt_add_qml_module(XY7320Host
    URI XY7320Host
    VERSION 1.0
    QML_FILES
        Main.qml
        ${QML_COMPONENTS}
        ${QML_PAGES}
)

qt_add_resources(APP_RESOURCES
    src.qrc
)

target_sources(XY7320Host PRIVATE ${APP_RESOURCES})

target_link_libraries(XY7320Host
    PRIVATE
        Qt6::Quick
        Qt6::SerialPort
)

set_target_properties(XY7320Host PROPERTIES
    WIN32_EXECUTABLE TRUE
)
