import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic
import QtQuick.Dialogs
import QtQuick.Layouts
import XY7320Host

ApplicationWindow {
    id: window

    width: 1280
    height: 720
    minimumWidth: 1100
    minimumHeight: 680
    visible: true
    title: qsTr("XY7320 上位机")
    flags: Qt.Window | Qt.FramelessWindowHint
    color: "transparent"

    property int currentPage: 0
    readonly property int resizeMargin: 6
    readonly property int windowRadius: maximized ? 0 : 14
    readonly property bool maximized: visibility === Window.Maximized || visibility === Window.FullScreen

    FontLoader {
        id: iconFont
        source: "qrc:/xy7320host/assets/fonts/Font Awesome 6 Free-Solid-900.otf"
    }

    ETheme {
        id: theme
    }

    FileDialog {
        id: firmwareDialog
        title: qsTr("选择 APP 固件 bin")
        nameFilters: [qsTr("固件文件 (*.bin)"), qsTr("所有文件 (*)")]
        onAccepted: firmwareUploader.setFileUrl(selectedFile)
    }

    EAlertDialog {
        id: startDialog
        title: qsTr("确认开始升级")
        message: qsTr("即将通过当前串口发送 APP 固件。升级过程中请不要断开设备电源或串口连接。")
        cancelText: qsTr("取消")
        confirmText: qsTr("开始")
        onConfirm: firmwareUploader.start()
    }

    EAlertDialog {
        id: exitDialog
        title: qsTr("要退出应用吗？")
        message: qsTr("退出将关闭上位机窗口。")
        cancelText: qsTr("取消")
        confirmText: qsTr("退出")
        dismissOnOverlay: false
        onConfirm: window.close()
    }

    EToast {
        id: toast
        theme: theme
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 68 + yOffset
    }

    Item {
        anchors.fill: parent

        Rectangle {
            id: windowSurface
            anchors.fill: parent
            radius: window.windowRadius
            color: theme.primaryColor
            clip: !window.maximized

            ColumnLayout {
                id: contentWrapper
                anchors.fill: parent
                spacing: 0

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 52
                radius: window.windowRadius
                antialiasing: true
                color: theme.secondaryColor

                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: window.windowRadius
                    color: parent.color
                    visible: !window.maximized
                }

                MouseArea {
                    anchors.fill: parent
                    property point pressPos
                    property bool dragging: false
                    onPressed: function(mouse) {
                        pressPos = Qt.point(mouse.x, mouse.y)
                        dragging = false
                    }
                    onPositionChanged: function(mouse) {
                        if (window.maximized) return
                        var delta = Qt.point(mouse.x - pressPos.x, mouse.y - pressPos.y)
                        if (!dragging && (Math.abs(delta.x) > 2 || Math.abs(delta.y) > 2)) {
                            dragging = true
                        }
                        if (dragging) {
                            window.x += delta.x
                            window.y += delta.y
                        }
                    }
                    onReleased: {
                        if (!dragging && pressPos.y <= 52) {
                            if (window.visibility === Window.Maximized) {
                                window.showNormal()
                            }
                        }
                        dragging = false
                    }
                    onDoubleClicked: {
                        if (!window.maximized) {
                            window.showMaximized()
                        } else {
                            window.showNormal()
                        }
                    }
                }

                RowLayout {
                    anchors.left: parent.left
                    anchors.leftMargin: 18
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 10

                    Rectangle {
                        width: 26
                        height: 26
                        radius: 13
                        color: Qt.rgba(theme.focusColor.r, theme.focusColor.g, theme.focusColor.b, 0.16)

                        Text {
                            anchors.centerIn: parent
                            text: "\uf61f"
                            font.family: iconFont.name
                            font.pixelSize: 13
                            color: theme.focusColor
                        }
                    }

                    ColumnLayout {
                        spacing: 2

                        Text {
                            text: qsTr("XY7320 上位机")
                            color: theme.textColor
                            font.pixelSize: 15
                            font.bold: true
                        }

                        Text {
                            text: qsTr("串口固件升级工具")
                            color: Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 0.6)
                            font.pixelSize: 12
                        }
                    }
                }

                Rectangle {
                    anchors.right: parent.right
                    width: 1
                    height: parent.height
                    color: theme.borderColor
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 0

                Rectangle {
                    Layout.fillHeight: true
                    Layout.preferredWidth: 220
                    color: theme.secondaryColor

                    Rectangle {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        height: window.windowRadius
                        color: parent.color
                        visible: !window.maximized
                    }

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 8

                        Text {
                            text: qsTr("功能")
                            color: Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 0.55)
                            font.pixelSize: 12
                            font.bold: true
                            Layout.leftMargin: 8
                            Layout.preferredHeight: 28
                            verticalAlignment: Text.AlignVCenter
                        }

                    Repeater {
                        model: [
                            { name: qsTr("固件升级"), icon: "\uf1c6", page: 0, enabled: true },
                            { name: qsTr("串口调试"), icon: "\uf120", page: 1, enabled: true },
                            { name: qsTr("升级日志"), icon: "\uf15c", page: 2, enabled: true },
                            { name: qsTr("设置"), icon: "\uf013", page: 3, enabled: true },
                            { name: qsTr("设备监控"), icon: "\uf1e6", page: 4, enabled: false },
                            { name: qsTr("参数配置"), icon: "\uf085", page: 5, enabled: false },
                            { name: qsTr("系统维护"), icon: "\uf0ad", page: 6, enabled: false }
                        ]

                        delegate: Rectangle {
                            id: navItem
                            Layout.fillWidth: true
                            Layout.preferredHeight: 40
                            radius: 8
                            color: window.currentPage === modelData.page
                                   ? Qt.rgba(theme.focusColor.r, theme.focusColor.g, theme.focusColor.b, 0.16)
                                   : mouseArea.containsMouse
                                     ? Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 0.06)
                                     : "transparent"

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 14
                                anchors.rightMargin: 10
                                spacing: 12

                                Text {
                                    text: modelData.icon
                                    font.family: iconFont.name
                                    font.pixelSize: 15
                                    color: modelData.enabled
                                           ? (window.currentPage === modelData.page ? theme.focusColor : theme.textColor)
                                           : Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 0.34)
                                    horizontalAlignment: Text.AlignHCenter
                                    Layout.preferredWidth: 20
                                }

                                Text {
                                    text: modelData.name
                                    color: modelData.enabled
                                           ? (window.currentPage === modelData.page ? theme.focusColor : theme.textColor)
                                           : Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 0.34)
                                    font.pixelSize: 14
                                    font.bold: window.currentPage === modelData.page
                                    Layout.fillWidth: true
                                    elide: Text.ElideRight
                                }
                            }

                            MouseArea {
                                id: mouseArea
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: modelData.enabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor
                                onClicked: {
                                    if (modelData.enabled) {
                                        window.currentPage = modelData.page
                                    } else {
                                        toast.show(qsTr("该模块已预留，后续接入。"))
                                    }
                                }
                            }
                        }
                    }

                    Item {
                        Layout.fillHeight: true
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 86
                        radius: 8
                        color: Qt.rgba(theme.focusColor.r, theme.focusColor.g, theme.focusColor.b, 0.10)

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 4

                            Text {
                                text: qsTr("项目")
                                color: Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 0.55)
                                font.pixelSize: 12
                                font.bold: true
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                            }

                            Text {
                                text: qsTr("XY7320 固件升级")
                                color: theme.textColor
                                font.pixelSize: 13
                                font.bold: true
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                            }

                            Text {
                                text: qsTr("版本 1.0.0 · 串口协议")
                                color: Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 0.55)
                                font.pixelSize: 11
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                            }
                        }
                    }
                }

                Rectangle {
                    anchors.right: parent.right
                    width: 1
                    height: parent.height
                    color: theme.borderColor
                }
            }

            StackLayout {
                id: stack
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: window.currentPage

                UpgradePage {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    firmwareDialog: firmwareDialog
                    startDialog: startDialog
                    toast: toast
                }

                SerialDebugPage {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                LogsPage {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                SettingsPage {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    toast: toast
                }
            }
        }
        }
        }

        Item {
            id: windowBorderLayer
            anchors.fill: parent
            z: 1050
            visible: !window.maximized

            Rectangle {
                anchors.fill: parent
                color: "transparent"
                radius: window.windowRadius
                antialiasing: true
                border.width: 1
                border.color: Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 0.22)
            }

            Rectangle {
                anchors.fill: parent
                anchors.margins: 1
                color: "transparent"
                radius: Math.max(0, window.windowRadius - 1)
                antialiasing: true
                border.width: 1
                border.color: Qt.rgba(theme.borderColor.r, theme.borderColor.g, theme.borderColor.b, 0.78)
            }
        }

        EBlurCard {
            id: titleButtonsPanel
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.rightMargin: 8
            anchors.topMargin: 4
            width: titleButtonsRow.implicitWidth + 14
            height: 40
            borderRadius: 20
            blurSource: contentWrapper
            blurAmount: 1.2
            blurMax: 32
            borderColor: Qt.rgba(theme.borderColor.r, theme.borderColor.g, theme.borderColor.b, 0.78)
            borderWidth: 1.2
            z: 1100

            Row {
                id: titleButtonsRow
                anchors.fill: parent
                anchors.margins: 7
                spacing: 8

                EButton {
                    width: 30
                    height: 26
                    radius: 13
                    backgroundVisible: true
                    shadowEnabled: false
                    text: ""
                    iconCharacter: "\uf068"
                    containerColor: Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 0.06)
                    hoverColor: Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 0.12)
                    onClicked: window.showMinimized()
                }

                EButton {
                    width: 30
                    height: 26
                    radius: 13
                    backgroundVisible: true
                    shadowEnabled: false
                    text: ""
                    iconCharacter: window.maximized ? "\uf2d2" : "\uf2d0"
                    containerColor: Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 0.06)
                    hoverColor: Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 0.12)
                    onClicked: {
                        if (window.maximized) {
                            window.showNormal()
                        } else {
                            window.showMaximized()
                        }
                    }
                }

                EButton {
                    width: 30
                    height: 26
                    radius: 13
                    backgroundVisible: true
                    shadowEnabled: false
                    text: ""
                    iconCharacter: "\uf2d5"
                    containerColor: theme.focusColor
                    hoverColor: Qt.darker(theme.focusColor, 1.12)
                    textColor: "white"
                    iconColor: "white"
                    onClicked: exitDialog.open()
                }
            }
        }

        Item {
            id: resizeHandle
            anchors.fill: parent
            z: 1200
            visible: !window.maximized

            property int edges: 0

            function updateEdges(x, y) {
                var e = 0
                if (x < resizeMargin) e |= 1
                if (x >= width - resizeMargin) e |= 2
                if (y < resizeMargin) e |= 4
                if (y >= height - resizeMargin) e |= 8
                edges = e
            }

            function cursorForEdges(e) {
                if (e === 0) return Qt.ArrowCursor
                if (e === 1 || e === 2) return Qt.SizeHorCursor
                if (e === 4 || e === 8) return Qt.SizeVerCursor
                if (e === 5 || e === 10) return Qt.SizeFDiagCursor
                if (e === 6 || e === 9) return Qt.SizeBDiagCursor
                return Qt.ArrowCursor
            }

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton
                propagateComposedEvents: true
                hoverEnabled: true

                property bool resizing: false
                property point startPos
                property rect startRect

                onPressed: function(mouse) {
                    if (resizeHandle.edges === 0) {
                        mouse.accepted = false
                        return
                    }
                    resizing = true
                    startPos = Qt.point(mouse.x, mouse.y)
                    startRect = Qt.rect(window.x, window.y, window.width, window.height)
                    mouse.accepted = true
                }

                onPositionChanged: function(mouse) {
                    resizeHandle.updateEdges(mouse.x, mouse.y)
                    cursorShape = resizeHandle.cursorForEdges(resizeHandle.edges)
                    
                    if (!resizing) {
                        mouse.accepted = false
                        return
                    }
                    var dx = mouse.x - startPos.x
                    var dy = mouse.y - startPos.y
                    var newRect = startRect
                    var e = resizeHandle.edges

                    if (e & 1) {
                        newRect.x = startRect.x + dx
                        newRect.width = Math.max(window.minimumWidth, startRect.width - dx)
                    }
                    if (e & 2) {
                        newRect.width = Math.max(window.minimumWidth, startRect.width + dx)
                    }
                    if (e & 4) {
                        newRect.y = startRect.y + dy
                        newRect.height = Math.max(window.minimumHeight, startRect.height - dy)
                    }
                    if (e & 8) {
                        newRect.height = Math.max(window.minimumHeight, startRect.height + dy)
                    }

                    window.setX(newRect.x)
                    window.setY(newRect.y)
                    window.setWidth(newRect.width)
                    window.setHeight(newRect.height)
                    mouse.accepted = true
                }

                onReleased: {
                    resizing = false
                    mouse.accepted = true
                }

                onContainsMouseChanged: {
                    if (!containsMouse) {
                        cursorShape = Qt.ArrowCursor
                    }
                }
            }
        }
    }

    Shortcut {
        sequence: "Esc"
        onActivated: {
            if (window.maximized) {
                window.showNormal()
            }
        }
    }
}
