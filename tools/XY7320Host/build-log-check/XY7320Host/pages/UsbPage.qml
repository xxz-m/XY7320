import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Basic
import XY7320Host

Item {
    id: root

    readonly property color mutedText:
        Qt.rgba(
            theme.textColor.r,
            theme.textColor.g,
            theme.textColor.b,
            0.62
        )

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 28
        spacing: 16

        ECard {
            Layout.fillWidth: true
            padding: 20
            radius: 10
            shadowEnabled: false
            cardColor: theme.instrumentPanelColor
            borderColor: theme.instrumentBorderColor
            borderWidth: 1

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 16

                Text {
                    text: qsTr("固件包")
                    color: theme.textColor
                    font.pixelSize: 15
                    font.bold: true
                    Layout.fillWidth: true
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 132
                    radius: 8
                    color: theme.instrumentInsetColor
                    border.color: theme.instrumentBorderColor
                    border.width: 1

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 16

                        Text {
                            text: "\uf1c6"
                            font.family: "Font Awesome 6 Free"
                            font.pixelSize: 40
                            color: theme.focusColor
                            Layout.preferredWidth: 54
                            horizontalAlignment: Text.AlignHCenter
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 5

                            Text {
                                text: qsTr("未检测到固件包")
                                color: theme.textColor
                                font.pixelSize: 15
                                font.bold: true
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                            }

                            Text {
                                text: qsTr("请刷新 U 盘中的固件包")
                                color: root.mutedText
                                font.pixelSize: 12
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                            }
                        }

                        EButton {
                            text: qsTr("刷新")
                            onClicked: usbManager.refreshDrives()
                            iconCharacter: "\uf021"
                            size: "xs"
                            radius: 8
                            shadowEnabled: false
                            Layout.preferredWidth: 100
                            Layout.preferredHeight: 40
                        }

                    }
                }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12
                    InfoBox {
                        label: qsTr("版本")
                        value: qsTr("未设置")
                        Layout.fillWidth: true
                    }

                    InfoBox {
                        label: qsTr("文件大小")
                        value: qsTr("-")
                        Layout.fillWidth: true
                    }
                }
            }
        }
        ECard {
            Layout.fillWidth: true
            Layout.preferredHeight: 300
            radius: 10
            padding: 0
            shadowEnabled: false

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 0

                // 工具栏
                RowLayout {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40
                    Layout.leftMargin: 14
                    Layout.rightMargin: 14
                    spacing: 10

                    Text {
                        text: qsTr("文件目录")
                        color: theme.textColor
                        font.pixelSize: 14
                        font.bold: true
                    }

                    Item { Layout.fillWidth: true }

                    ECheckBox {
                        text: qsTr("自动滚动")
                        checked: serialDebug.autoScroll
                        onToggled: serialDebug.autoScroll = checked
                    }
                    EButton {
                        text: qsTr("清空")
                        iconCharacter: "\uf2ed"
                        size: "xs"
                        radius: 8
                        shadowEnabled: false
                        Layout.preferredWidth: 80
                        Layout.preferredHeight: 32
                        onClicked: serialDebug.clear()
                    }
                }

                // 日志显示区
                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: usbManager.files

                    delegate: Rectangle {
                        width: ListView.view.width
                        height: 64
                        radius: 8
                        color: theme.instrumentInsetColor

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 12

                            Text {
                                text: "\uf1c6"
                                font.family: "Font Awesome 6 Free"
                                font.pixelSize: 20
                                color: theme.focusColor
                                Layout.preferredWidth: 28
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2

                                Text {
                                    text: modelData.name
                                    color: theme.textColor
                                    font.bold: true
                                    Layout.fillWidth: true
                                    elide: Text.ElideRight
                                }

                                Text {
                                    text: modelData.modifiedText
                                    color: root.mutedText
                                    font.pixelSize: 11
                                }
                            }

                            Text {
                                text: modelData.sizeText
                                color: root.mutedText
                            }
                        }
                    }
                }
            }
        }

    }
    component InfoBox: Rectangle {
        id: infoBox
        property string label: ""
        property string value: ""
        Layout.preferredHeight: 56
        radius: 8
        color: theme.instrumentInsetColor
        border.color: theme.instrumentBorderColor
        border.width: 1
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 3
            Text { text: infoBox.label; color: root.mutedText; font.pixelSize: 11 }
            Text { text: infoBox.value; color: theme.textColor; font.pixelSize: 13; font.bold: true; elide: Text.ElideRight; Layout.fillWidth: true }
        }
    }
}