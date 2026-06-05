cmake_minimum_required(VERSION 3.20)

project(QtUpgradeDemo VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick SerialPort)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(QtUpgradeDemo
    main.cpp
    firmwareuploader.cpp
    firmwareuploader.h
)

qt_add_qml_module(QtUpgradeDemo
    URI QtUpgradeDemo
    VERSION 1.0
    QML_FILES
        Main.qml
)

target_link_libraries(QtUpgradeDemo
    PRIVATE
        Qt6::Quick
        Qt6::SerialPort
)
