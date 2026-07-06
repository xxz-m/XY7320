pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import SerialAssistant

ApplicationWindow {
    id: window
    width: 1440
    height: 900
    minimumWidth: 1100
    minimumHeight: 700
    visible: true
    title: qsTr("SerialAssistant")
    flags: Qt.Window | Qt.FramelessWindowHint
    color: "transparent"

    property int resizeHandleSize: 8
    property int frameRadius: 18

    ETheme {
        id: appTheme
        isDark: settingsManager.themeMode === 1
        focusColor: settingsManager.accentColor
    }

    property bool enterToSend: settingsManager.enterToSend
    property bool keywordFilter: false
    property string keywordText: ""
    property bool settingsExpanded: true
    property bool settingsPanelVisible: false
    property string currentSettingsSection: "serial"

    onClosing: serialController.closePort()

    Rectangle {
        id: appFrame
        anchors.fill: parent
        anchors.margins: 1
        radius: window.frameRadius
        color: appTheme.primaryColor
        border.width: 1
        border.color: appTheme.borderColor
        clip: true

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 16 + window.resizeHandleSize
            spacing: 12

            WindowTopBar {
                Layout.fillWidth: true
                Layout.preferredHeight: 46
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 16

                // 左侧导航与过滤组件包
            Rectangle {
                Layout.preferredWidth: 270
                Layout.fillHeight: true
                radius: 14
                color: appTheme.secondaryColor
                border.width: 1
                border.color: appTheme.borderColor

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 14
                    spacing: 12

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 70
                        spacing: 12
                        Rectangle {
                            Layout.preferredWidth: 48
                            Layout.preferredHeight: 48
                            radius: 12
                            clip: true
                            color: appTheme.elevatedColor
                            border.width: 1
                            border.color: appTheme.borderColor

                            Image {
                                anchors.fill: parent
                                source: Qt.resolvedUrl("assets/icons/app_icon.png")
                                fillMode: Image.PreserveAspectCrop
                                smooth: true
                                mipmap: true
                            }
                        }
                        ColumnLayout {
                            spacing: 2
                            Text { text: "SerialAssistant"; color: appTheme.textColor; font.pixelSize: 16; font.bold: true }
                            Text { text: "v0.2.0 · MIT"; color: appTheme.mutedTextColor; font.pixelSize: 11 }
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 5
                        SidebarItem { text: qsTr("控制台"); active: true; iconName: "terminal" }
                        SidebarItem { text: qsTr("数据波形"); iconName: "monitoring" }
                        SidebarItem { text: qsTr("协议解析"); iconName: "description" }
                        SidebarItem {
                            text: qsTr("设置")
                            iconName: "settings"
                            trailingIcon: window.settingsExpanded ? "expand_less" : "expand_more"
                            onActivated: window.settingsExpanded = !window.settingsExpanded
                        }
                    }

                    // 设置使用层级菜单，不与底部发送选项重复。
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: settingsColumn.implicitHeight + 20
                        visible: window.settingsExpanded
                        radius: 10
                        color: appTheme.isDark ? appTheme.elevatedColor : "#FBFCFE"
                        border.width: 1
                        border.color: appTheme.borderColor

                        ColumnLayout {
                            id: settingsColumn
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.top: parent.top
                            anchors.margins: 10
                            spacing: 3

                            SettingsRow { text: qsTr("串口设置"); iconName: "settings_input_component"; selected: window.settingsPanelVisible && window.currentSettingsSection === "serial"; onActivated: window.openSettings("serial") }
                            SettingsRow { text: qsTr("编码规则"); iconName: "translate"; selected: window.settingsPanelVisible && window.currentSettingsSection === "encoding"; onActivated: window.openSettings("encoding") }
                            SettingsRow { text: qsTr("回车样式"); iconName: "keyboard_return"; selected: window.settingsPanelVisible && window.currentSettingsSection === "lineEnding"; onActivated: window.openSettings("lineEnding") }
                            SettingsRow { text: qsTr("个性化"); iconName: "palette"; selected: window.settingsPanelVisible && window.currentSettingsSection === "appearance"; onActivated: window.openSettings("appearance") }
                            SettingsRow { text: qsTr("关键字高亮"); iconName: "format_color_fill"; selected: window.settingsPanelVisible && window.currentSettingsSection === "highlight"; onActivated: window.openSettings("highlight") }
                            SettingsRow { text: qsTr("时间戳设置"); iconName: "schedule"; selected: window.settingsPanelVisible && window.currentSettingsSection === "timestamp"; onActivated: window.openSettings("timestamp") }
                            Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: appTheme.borderColor }
                            SettingsRow { text: qsTr("恢复默认设置"); iconName: "restart_alt"; selected: window.settingsPanelVisible && window.currentSettingsSection === "restore"; onActivated: window.openSettings("restore") }
                        }
                    }

                    Item { Layout.fillHeight: true }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 54
                        radius: 9
                        color: appTheme.elevatedColor
                        border.width: 1
                        border.color: appTheme.borderColor
                        RowLayout {
                            anchors.fill: parent; anchors.margins: 12; spacing: 8
                            Rectangle {
                                Layout.preferredWidth: 9; Layout.preferredHeight: 9; radius: 5
                                color: serialController.isOpen ? appTheme.successColor : appTheme.mutedTextColor
                            }
                            Text {
                                Layout.fillWidth: true
                                text: serialController.isOpen ? qsTr("串口已连接") : qsTr("串口未连接")
                                color: appTheme.textColor; font.pixelSize: 12
                            }
                        }
                    }
                }
            }

            // 右侧工作区
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 14

                // 顶部串口连接卡片
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 112
                    radius: 14
                    color: appTheme.secondaryColor
                    border.width: 1
                    border.color: appTheme.borderColor

                    ColumnLayout {
                        anchors.fill: parent; anchors.margins: 16; spacing: 10
                        Text { text: qsTr("串口连接"); color: appTheme.textColor; font.pixelSize: 14; font.bold: true }
                        RowLayout {
                            Layout.fillWidth: true; spacing: 12
                            Text { text: qsTr("端口"); color: appTheme.mutedTextColor; font.pixelSize: 12 }
                            EDropdown {
                                theme: appTheme; Layout.fillWidth: true; Layout.preferredHeight: 38
                                model: serialController.ports.length > 0 ? serialController.ports : [qsTr("无可用端口")]
                                selectedValue: serialController.ports.length > 0 ? serialController.portName : qsTr("无可用端口")
                                enabled: serialController.ports.length > 0
                                onValueSelected: value => serialController.portName = value
                            }
                            Text { text: qsTr("波特率"); color: appTheme.mutedTextColor; font.pixelSize: 12 }
                            EDropdown {
                                theme: appTheme; Layout.preferredWidth: 190; Layout.preferredHeight: 38
                                model: serialController.baudRateOptions; selectedValue: serialController.baudRate
                                onValueSelected: value => serialController.baudRate = value
                            }
                            EButton {
                                theme: appTheme; text: qsTr("刷新"); iconName: "refresh"
                                Layout.preferredWidth: 100; Layout.preferredHeight: 38
                                onClicked: serialController.refreshPorts()
                            }
                            EButton {
                                theme: appTheme
                                text: serialController.isOpen ? qsTr("关闭串口") : qsTr("打开串口")
                                iconName: serialController.isOpen ? "stop_circle" : "play_arrow"
                                primary: true
                                containerColor: serialController.isOpen ? appTheme.dangerColor : appTheme.focusColor
                                Layout.preferredWidth: 160; Layout.preferredHeight: 38
                                enabled: serialController.isOpen || serialController.portName.length > 0
                                onClicked: serialController.isOpen ? serialController.closePort() : serialController.openPort()
                            }
                        }
                    }
                }

                ReceivePaneCard {
                    theme: appTheme
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    recordModel: consoleController.records
                    timestampEnabled: settingsManager.timestampEnabled
                    timestampScope: settingsManager.timestampScope
                    autoScroll: settingsManager.autoScroll
                    receivePaused: consoleController.receivePaused
                    keywordFilter: window.keywordFilter
                    keywordHighlight: settingsManager.highlightEnabled
                    highlightCaseSensitive: settingsManager.highlightCaseSensitive
                    highlightRegex: settingsManager.highlightRegex
                    keywordText: window.keywordText
                    hexDisplay: settingsManager.hexDisplay
                    terminalFont: settingsManager.terminalFont
                    terminalFontSize: settingsManager.terminalFontSize
                    uiDensity: settingsManager.uiDensity
                    onPauseToggled: consoleController.receivePaused = !consoleController.receivePaused
                    onCleared: consoleController.clear()
                }

                SendPaneCard {
                    id: sendCard
                    theme: appTheme
                    Layout.fillWidth: true
                    Layout.preferredHeight: 176
                    Layout.minimumHeight: 160
                    isPortOpen: serialController.isOpen
                    byteCount: sendText.length
                    hexDisplay: settingsManager.hexDisplay
                    hexSend: transmitController.hexSend
                    loopSend: transmitController.loopSend
                    loopIntervalMs: transmitController.loopIntervalMs
                    enterToSend: settingsManager.enterToSend
                    appendNewline: settingsManager.appendNewline
                    multilineInput: settingsManager.multilineInput
                    terminalFont: settingsManager.terminalFont
                    terminalFontSize: settingsManager.terminalFontSize
                    uiDensity: settingsManager.uiDensity
                    timestampEnabled: settingsManager.timestampEnabled
                    onCleared: sendText = ""
                    onSendRequested: transmitController.send(sendText)
                    onTimestampSettingChanged: enabled => settingsManager.timestampEnabled = enabled
                    onSendSettingChanged: (name, value) => {
                        if (name === "hexSend") transmitController.hexSend = value
                        else if (name === "loopSend") {
                            if (value) transmitController.startLoopSend(sendText)
                            else transmitController.stopLoopSend()
                        }
                        else if (name === "loopIntervalMs") transmitController.loopIntervalMs = value
                        else if (name === "enterToSend") settingsManager.enterToSend = value
                        else if (name === "appendNewline") {
                            settingsManager.appendNewline = value
                            transmitController.lineEnding = value ? 3 : 0
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 26
                    spacing: 16
                    Item { Layout.fillWidth: true }
                    StatusChip { text: qsTr("RX %1 B · %2 帧").arg(consoleController.rxBytes).arg(consoleController.rxFrames) }
                    StatusChip { text: qsTr("TX %1 B · %2 帧").arg(transmitController.txBytes).arg(transmitController.txFrames) }
                    StatusChip { text: transmitController.errorText.length > 0 ? transmitController.errorText : qsTr("就绪"); accent: transmitController.errorText.length === 0 }
                }
            }
        }
        }

        Rectangle {
            anchors.fill: parent
            visible: window.settingsPanelVisible
            color: "#26000000"
            z: 20
            MouseArea { anchors.fill: parent; onClicked: window.settingsPanelVisible = false }
        }

        SettingsPanel {
            id: settingsPanel
            theme: appTheme
            settingsManager: settingsManager
            serialController: serialController
            transmitController: transmitController
            visible: window.settingsPanelVisible
            currentSection: window.currentSettingsSection
            width: Math.min(440, window.width - 40)
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.margins: 16 + window.resizeHandleSize
            z: 21
            onCloseRequested: window.settingsPanelVisible = false

        }

        MoveHandle { anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top; anchors.leftMargin: window.resizeHandleSize; anchors.rightMargin: window.resizeHandleSize; height: window.resizeHandleSize }
        MoveHandle { anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom; anchors.leftMargin: window.resizeHandleSize; anchors.rightMargin: window.resizeHandleSize; height: window.resizeHandleSize }
        MoveHandle { anchors.left: parent.left; anchors.top: parent.top; anchors.bottom: parent.bottom; anchors.topMargin: window.resizeHandleSize; anchors.bottomMargin: window.resizeHandleSize; width: window.resizeHandleSize }
        MoveHandle { anchors.right: parent.right; anchors.top: parent.top; anchors.bottom: parent.bottom; anchors.topMargin: window.resizeHandleSize; anchors.bottomMargin: window.resizeHandleSize; width: window.resizeHandleSize }
        ResizeHandle { edge: Qt.TopEdge | Qt.LeftEdge; anchors.left: parent.left; anchors.top: parent.top; width: window.resizeHandleSize; height: window.resizeHandleSize; resizeCursor: Qt.SizeFDiagCursor }
        ResizeHandle { edge: Qt.TopEdge | Qt.RightEdge; anchors.right: parent.right; anchors.top: parent.top; width: window.resizeHandleSize; height: window.resizeHandleSize; resizeCursor: Qt.SizeBDiagCursor }
        ResizeHandle { edge: Qt.BottomEdge | Qt.LeftEdge; anchors.left: parent.left; anchors.bottom: parent.bottom; width: window.resizeHandleSize; height: window.resizeHandleSize; resizeCursor: Qt.SizeBDiagCursor }
        ResizeHandle { edge: Qt.BottomEdge | Qt.RightEdge; anchors.right: parent.right; anchors.bottom: parent.bottom; width: window.resizeHandleSize; height: window.resizeHandleSize; resizeCursor: Qt.SizeFDiagCursor }
    }

    function openSettings(section) {
        currentSettingsSection = section
        settingsPanelVisible = true
    }

    component MoveHandle: MouseArea {
        z: 100
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton
        cursorShape: Qt.SizeAllCursor
        onPressed: window.startSystemMove()
    }

    component ResizeHandle: MouseArea {
        id: resizeHandle
        required property int edge
        property int resizeCursor: Qt.ArrowCursor

        z: 101
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton
        cursorShape: resizeHandle.resizeCursor
        onPressed: window.startSystemResize(edge)
    }

    component WindowTopBar: Rectangle {
        id: topBar
        radius: height / 2
        color: appTheme.titleBarColor
        border.width: 1
        border.color: appTheme.borderColor

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 16
            anchors.rightMargin: 10
            spacing: 6

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                Text {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("SerialAssistant")
                    color: appTheme.textColor
                    font.pixelSize: 13
                    font.bold: true
                    elide: Text.ElideRight
                }
            }

            WindowToolButton {
                iconName: settingsManager.themeMode === 1 ? "light_mode" : "dark_mode"
                onClicked: settingsManager.themeMode = settingsManager.themeMode === 1 ? 0 : 1
            }

            WindowToolButton {
                iconName: "remove"
                onClicked: window.showMinimized()
            }

            WindowToolButton {
                iconName: "close"
                danger: true
                onClicked: window.close()
            }
        }
    }

    component WindowToolButton: Rectangle {
        id: button
        property string iconName: "circle"
        property bool danger: false
        signal clicked()

        Layout.preferredWidth: 44
        Layout.preferredHeight: 34
        Layout.alignment: Qt.AlignVCenter
        radius: height / 2
        color: mouse.pressed
               ? (button.danger ? appTheme.dangerColor : appTheme.windowButtonPressedColor)
               : (mouse.containsMouse
                  ? (button.danger ? appTheme.dangerColor : appTheme.windowButtonHoverColor)
                  : Qt.rgba(appTheme.elevatedColor.r, appTheme.elevatedColor.g, appTheme.elevatedColor.b, 0.78))
        border.width: 1
        border.color: mouse.containsMouse ? appTheme.focusColor : appTheme.borderColor

        MaterialIcon {
            anchors.centerIn: parent
            icon: button.iconName
            color: button.danger && mouse.containsMouse ? "#FFFFFF" : appTheme.mutedTextColor
            font.pixelSize: 18
        }

        MouseArea {
            id: mouse
            anchors.fill: parent
            z: 10
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton
            preventStealing: true
            cursorShape: Qt.PointingHandCursor
            onClicked: button.clicked()
        }
    }

    component SidebarItem: Rectangle {
        id: navItem
        property string text: ""
        property string iconName: "circle"
        property bool active: false
        property string trailingIcon: ""
        signal activated()
        Layout.fillWidth: true; Layout.preferredHeight: 46
        radius: 9
        color: active ? Qt.rgba(appTheme.focusColor.r, appTheme.focusColor.g, appTheme.focusColor.b, 0.12)
                      : (navMouse.containsMouse ? appTheme.elevatedColor : "transparent")
        RowLayout {
            anchors.fill: parent; anchors.leftMargin: 14; anchors.rightMargin: 14; spacing: 12
            MaterialIcon {
                icon: navItem.iconName
                color: navItem.active ? appTheme.focusColor : appTheme.mutedTextColor
                font.pixelSize: 20
                Layout.preferredWidth: 22
            }
            Text { text: navItem.text; color: navItem.active ? appTheme.focusColor : appTheme.textColor; font.pixelSize: 13; font.bold: navItem.active }
            Item { Layout.fillWidth: true }
            MaterialIcon {
                icon: navItem.trailingIcon
                color: appTheme.mutedTextColor
                font.pixelSize: 22
                visible: icon.length > 0
                Layout.preferredWidth: 28
            }
        }
        MouseArea {
            id: navMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: navItem.activated()
        }
    }

    component SettingsRow: Rectangle {
        id: settingsRow
        property string text: ""
        property int level: 1
        property bool expandable: false
        property bool expanded: false
        property bool selected: false
        property string iconName: "settings"
        signal activated()
        Layout.fillWidth: true
        Layout.preferredHeight: level === 1 ? 32 : 28
        radius: 6
        color: selected ? Qt.rgba(appTheme.focusColor.r, appTheme.focusColor.g, appTheme.focusColor.b, 0.12)
                        : (settingsMouse.containsMouse ? appTheme.elevatedColor : "transparent")
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: settingsRow.level === 1 ? 10 : 28
            anchors.rightMargin: 10
            spacing: 7
            MaterialIcon {
                icon: settingsRow.iconName
                Layout.preferredWidth: 20
                color: settingsRow.selected ? appTheme.focusColor : appTheme.mutedTextColor
                font.pixelSize: 18
            }
            Text {
                Layout.fillWidth: true
                text: settingsRow.text
                color: settingsRow.selected ? appTheme.focusColor : appTheme.textColor
                font.pixelSize: settingsRow.level === 1 ? 12 : 11
                font.bold: settingsRow.selected
            }
            MaterialIcon {
                visible: true
                icon: "chevron_right"
                color: appTheme.mutedTextColor
                font.pixelSize: 20
                Layout.preferredWidth: 26
            }
        }
        MouseArea {
            id: settingsMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: settingsRow.activated()
        }
    }

    component StatusChip: Rectangle {
        property string text: ""
        property bool accent: false
        Layout.preferredWidth: chipText.implicitWidth + 24
        Layout.preferredHeight: 26
        radius: 7
        color: accent ? Qt.rgba(appTheme.focusColor.r, appTheme.focusColor.g, appTheme.focusColor.b, 0.1) : appTheme.secondaryColor
        border.width: 1; border.color: appTheme.borderColor
        Text { id: chipText; anchors.centerIn: parent; text: parent.text; color: parent.accent ? appTheme.focusColor : appTheme.mutedTextColor; font.pixelSize: 11 }
    }
}
