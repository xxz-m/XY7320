import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic
import QtQuick.Layouts
import XY7320Host

Item {
    id: root

    required property var toast

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 14

        Text {
            text: qsTr("设置")
            color: theme.textColor
            font.pixelSize: 22
            font.bold: true
            Layout.fillWidth: true
            Layout.preferredHeight: 32
            elide: Text.ElideRight
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 16

            ECard {
                Layout.preferredWidth: 420
                Layout.fillHeight: true
                radius: 10
                padding: 18
                shadowEnabled: false

                ColumnLayout {
                    spacing: 12

                    Text {
                        text: qsTr("界面")
                        color: theme.textColor
                        font.pixelSize: 16
                        font.bold: true
                        Layout.fillWidth: true
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10

                        Text {
                            text: qsTr("主题")
                            color: theme.textColor
                            font.pixelSize: 13
                            Layout.fillWidth: true
                        }

                        EButton {
                            text: theme.isDark ? qsTr("浅色") : qsTr("深色")
                            iconCharacter: theme.isDark ? "\uf185" : "\uf186"
                            size: "xs"
                            radius: 8
                            shadowEnabled: false
                            Layout.preferredWidth: 100
                            Layout.preferredHeight: 38
                            onClicked: {
                                theme.toggleTheme()
                                root.toast.show(qsTr("主题已切换。"))
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: theme.borderColor
                    }

                    Text {
                        text: qsTr("窗口默认 1280x720，最小 1100x680。控件高度保持在 36 到 44 像素，升级中不改变主操作区布局。")
                        color: Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 0.68)
                        font.pixelSize: 13
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }
                }
            }

            ECard {
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: 10
                padding: 18
                shadowEnabled: false

                ColumnLayout {
                    spacing: 12

                    Text {
                        text: qsTr("后续模块")
                        color: theme.textColor
                        font.pixelSize: 16
                        font.bold: true
                        Layout.fillWidth: true
                    }

                    EDataTable {
                        headers: [
                            { key: "name", label: qsTr("模块") },
                            { key: "status", label: qsTr("状态") },
                            { key: "note", label: qsTr("说明") }
                        ]
                        model: ListModel {
                            ListElement { name: "设备监控"; status: "预留"; note: "连接状态、在线信息、运行数据" }
                            ListElement { name: "参数配置"; status: "预留"; note: "设备参数读写和保存" }
                            ListElement { name: "系统维护"; status: "预留"; note: "诊断、复位、维护命令" }
                        }
                        selectable: false
                        radius: 8
                        rowHeight: 38
                        headerHeight: 42
                        shadowEnabled: false
                        Layout.fillWidth: true
                        Layout.preferredHeight: 190
                    }

                    Text {
                        text: qsTr("第一版只接入 APP bin 升级；YModem/mOTA .fpk 等完整协议后续再扩展。")
                        color: Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 0.68)
                        font.pixelSize: 13
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }
}
