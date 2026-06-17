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
                    onPressed: window.startSystemMove()
                    onDoubleClicked: window.maximized ? window.showNormal() : window.showMaximized()
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 20
                    anchors.rightMargin: titleButtonsPanel.width + 40
                    spacing: 14

                    Image {
                        source: "qrc:/xy7320host/assets/images/icon.ico"
                        sourceSize.width: 24
                        sourceSize.height: 24
                        fillMode: Image.PreserveAspectFit
                        Layout.preferredWidth: 24
                        Layout.preferredHeight: 24
                    }

                    Text {
                        text: qsTr("XY7320 上位机")
                        color: theme.textColor
                        font.pixelSize: 18
                        font.bold: true
                        Layout.preferredWidth: 160
                        elide: Text.ElideRight
                    }

                    Rectangle {
                        Layout.preferredWidth: 1
                        Layout.preferredHeight: 24
                        color: theme.borderColor
                    }

                    Text {
                        text: qsTr("APP 固件升级与上位机启动框架")
                        color: Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 0.72)
                        font.pixelSize: 13
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }

                    Text {
                        text: firmwareUploader.status
                        color: firmwareUploader.busy ? theme.focusColor : theme.textColor
                        font.pixelSize: 13
                        font.bold: firmwareUploader.busy
                        horizontalAlignment: Text.AlignRight
                        Layout.preferredWidth: 120
                        elide: Text.ElideRight
                    }

                    EButton {
                        text: ""
                        iconCharacter: theme.isDark ? "\uf185" : "\uf186"
                        size: "xs"
                        radius: 8
                        backgroundVisible: false
                        shadowEnabled: false
                        Layout.preferredWidth: 38
                        Layout.preferredHeight: 38
                        onClicked: theme.toggleTheme()
                    }
                }

                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: 1
                    color: theme.borderColor
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 0

                Rectangle {
                    Layout.preferredWidth: 220
                    Layout.fillHeight: true
                    radius: window.windowRadius
                    antialiasing: true
                    color: theme.secondaryColor

                    Rectangle {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        height: window.windowRadius
                        color: parent.color
                        visible: !window.maximized
                    }

                    Rectangle {
                        anchors.top: parent.top
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        width: window.windowRadius
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
                                text: qsTr("当前协议")
                                color: theme.focusColor
                                font.pixelSize: 12
                                font.bold: true
                                Layout.fillWidth: true
                            }

                            Text {
                                text: qsTr("12字节头 + APP bin")
                                color: theme.textColor
                                font.pixelSize: 12
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                            }

                            Text {
                                text: qsTr("APP: 0x08010000")
                                color: Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 0.66)
                                font.pixelSize: 12
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
                    containerColor: theme.primaryColor
                    hoverColor: Qt.rgba(theme.focusColor.r, theme.focusColor.g, theme.focusColor.b, 0.18)
                    onClicked: window.showMinimized()
                }

                EButton {
                    width: 30
                    height: 26
                    radius: 13
                    backgroundVisible: true
                    shadowEnabled: false
                    text: ""
                    iconCharacter: "\uf00d"
                    containerColor: theme.primaryColor
                    hoverColor: Qt.rgba(0.9, 0.18, 0.2, 0.16)
                    onClicked: exitDialog.open()
                }
            }
        }

        Item {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: window.resizeMargin
            z: 1000
            visible: !window.maximized

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.SizeHorCursor
                onPressed: window.startSystemResize(Qt.LeftEdge)
            }
        }

        Item {
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: window.resizeMargin
            z: 1000
            visible: !window.maximized

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.SizeHorCursor
                onPressed: window.startSystemResize(Qt.RightEdge)
            }
        }

        Item {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.rightMargin: titleButtonsPanel.width + 20
            height: window.resizeMargin
            z: 1000
            visible: !window.maximized

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.SizeVerCursor
                onPressed: window.startSystemResize(Qt.TopEdge)
            }
        }

        Item {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: window.resizeMargin
            z: 1000
            visible: !window.maximized

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.SizeVerCursor
                onPressed: window.startSystemResize(Qt.BottomEdge)
            }
        }

        Item {
            anchors.left: parent.left
            anchors.top: parent.top
            width: window.resizeMargin
            height: window.resizeMargin
            z: 1001
            visible: !window.maximized

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.SizeFDiagCursor
                onPressed: window.startSystemResize(Qt.LeftEdge | Qt.TopEdge)
            }
        }

        Item {
            anchors.right: parent.right
            anchors.top: parent.top
            width: window.resizeMargin
            height: window.resizeMargin
            z: 1001
            visible: !window.maximized

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.SizeBDiagCursor
                onPressed: window.startSystemResize(Qt.RightEdge | Qt.TopEdge)
            }
        }

        Item {
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            width: window.resizeMargin
            height: window.resizeMargin
            z: 1001
            visible: !window.maximized

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.SizeBDiagCursor
                onPressed: window.startSystemResize(Qt.LeftEdge | Qt.BottomEdge)
            }
        }

        Item {
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            width: window.resizeMargin
            height: window.resizeMargin
            z: 1001
            visible: !window.maximized

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.SizeFDiagCursor
                onPressed: window.startSystemResize(Qt.RightEdge | Qt.BottomEdge)
            }
        }
    }
}
