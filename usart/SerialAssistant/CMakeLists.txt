cmake_minimum_required(VERSION 3.20)

project(SerialAssistant VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.8 REQUIRED COMPONENTS Quick QuickControls2 SerialPort)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(SerialAssistant
    main.cpp
    app_icon.rc
        backend/io/serial_port_controller.cpp
        backend/io/serial_port_controller.h
        backend/settings/settings_manager.cpp
        backend/settings/settings_manager.h
        backend/console/console_controller.cpp
        backend/console/console_controller.h
        backend/console/transmit_controller.cpp
        backend/console/transmit_controller.h
        backend/console/text_codec.cpp
        backend/console/text_codec.h
        models/console_record_model.cpp
        models/console_record_model.h
)

qt_add_qml_module(SerialAssistant
    URI SerialAssistant
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/components/EOptRow.qml
        qml/components/EPaneCard.qml
        qml/components/EButton.qml
        qml/components/ECard.qml
        qml/components/EDropdown.qml
        qml/components/EInput.qml
        qml/components/ETheme.qml
        qml/components/MaterialIcon.qml
        qml/components/settings/SettingGroup.qml
        qml/components/settings/SettingSwitch.qml
        qml/components/settings/SegmentedSelector.qml
        qml/components/settings/SettingsPanel.qml
        qml/components/cards/ReceivePaneCard.qml
        qml/components/cards/FilterHighlightCard.qml
        qml/components/cards/SendPaneCard.qml
        qml/components/cards/SendSettingsCard.qml
    RESOURCES
        qml/assets/fonts/MaterialSymbolsRounded.ttf
        qml/assets/icons/app_icon.png
        qml/assets/icons/app_icon_256.png
        qml/assets/icons/app_icon.ico
)

target_link_libraries(SerialAssistant
    PRIVATE
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::SerialPort
)

target_compile_options(SerialAssistant
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
        $<$<CXX_COMPILER_ID:GNU>:-finput-charset=UTF-8>
        $<$<CXX_COMPILER_ID:GNU>:-fexec-charset=UTF-8>
)

set_target_properties(SerialAssistant PROPERTIES WIN32_EXECUTABLE TRUE)
