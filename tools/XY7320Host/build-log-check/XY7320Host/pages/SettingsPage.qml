import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic
import QtQuick.Layouts
import XY7320Host

Item {
    id: root

    required property var toast
    readonly property var baudRates: [
        { text: "9600" }, { text: "57600" }, { text: "115200" },
        { text: "230400" }, { text: "460800" }, { text: "921600" }
    ]

    function portModel() {
        var items = []
        for (var i = 0; i < firmwareUploader.ports.length; ++i)
            items.push({ text: firmwareUploader.ports[i].text, portName: firmwareUploader.ports[i].portName })
        return items
    }

    function currentPort() {
        for (var i = 0; i < firmwareUploader.ports.length; ++i)
            if (firmwareUploader.ports[i].portName === firmwareUploader.portName) return i
        return firmwareUploader.ports.length > 0 ? 0 : -1
    }

    function currentBaud() {
        for (var i = 0; i < baudRates.length; ++i)
            if (baudRates[i].text === String(firmwareUploader.baudRate)) return i
        return 2
    }

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth
        clip: true

        ColumnLayout {
            width: Math.max(parent.width, 860)
            anchors.margins: 20
            spacing: 14

            Text { text: qsTr("设置"); color: theme.textColor; font.pixelSize: 22; font.bold: true; Layout.leftMargin: 20; Layout.rightMargin: 20; Layout.fillWidth: true; Layout.preferredHeight: 32 }

            ECard {
                id: connectionCard
                z: (portDropdown.opened || baudDropdown.opened) ? 10 : 0
                Layout.fillWidth: true
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                radius: 10
                padding: 18
                shadowEnabled: false
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 12
                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: qsTr("参数配置 · 设备连接"); color: theme.textColor; font.pixelSize: 16; font.bold: true; Layout.fillWidth: true }
                        EButton { text: qsTr("刷新串口"); iconCharacter: "\uf021"; size: "xs"; radius: 8; shadowEnabled: false; enabled: !firmwareUploader.busy; Layout.preferredWidth: 108; Layout.preferredHeight: 36; onClicked: firmwareUploader.refreshPorts() }
                    }
                    Text { text: qsTr("该串口是固件升级和串口调试共用的唯一设备串口。升级过程中串口由升级流程独占。") ; color: Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 0.62); font.pixelSize: 12; Layout.fillWidth: true; wrapMode: Text.WordWrap }
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12
                        Text { text: qsTr("串口"); color: theme.textColor; font.pixelSize: 13; Layout.preferredWidth: 64 }
                        EDropdown { id: portDropdown; enabled: !firmwareUploader.busy && !firmwareUploader.serialOpen; model: root.portModel(); selectedIndex: root.currentPort(); title: qsTr("选择串口"); headerHeight: 40; radius: 8; fontSize: 13; popupMaxHeight: 280; scrollBarAlwaysVisible: true; shadowEnabled: false; Layout.fillWidth: true; Layout.preferredHeight: 40; onSelectionChanged: function(index, item) { firmwareUploader.portName = item.portName } }
                        Text { text: firmwareUploader.serialOpen ? qsTr("已连接") : qsTr("未连接"); color: firmwareUploader.serialOpen ? theme.focusColor : Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 0.58); font.pixelSize: 12; font.bold: firmwareUploader.serialOpen; Layout.preferredWidth: 60; horizontalAlignment: Text.AlignRight }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12
                        Text { text: qsTr("波特率"); color: theme.textColor; font.pixelSize: 13; Layout.preferredWidth: 64 }
                        EDropdown { id: baudDropdown; enabled: !firmwareUploader.busy && !firmwareUploader.serialOpen; model: root.baudRates; selectedIndex: root.currentBaud(); title: qsTr("115200"); headerHeight: 40; radius: 8; fontSize: 13; popupMaxHeight: 220; shadowEnabled: false; Layout.fillWidth: true; Layout.preferredHeight: 40; onSelectionChanged: function(index, item) { firmwareUploader.baudRate = Number(item.text) } }
                        ESwitchButton { text: firmwareUploader.serialOpen ? qsTr("关闭串口") : qsTr("打开串口"); checked: firmwareUploader.serialOpen; autoToggle: false; enabled: !firmwareUploader.busy && firmwareUploader.portName.length > 0; size: "s"; radius: 8; fontSize: 13; shadowEnabled: false; Layout.preferredWidth: 120; Layout.preferredHeight: 40; onToggled: function(nextChecked) { if (nextChecked) firmwareUploader.openPort(); else firmwareUploader.closePort() } }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                Layout.bottomMargin: 20
                spacing: 16
                ECard {
                    Layout.preferredWidth: 360
                    Layout.fillHeight: true
                    radius: 10
                    padding: 18
                    shadowEnabled: false
                    ColumnLayout {
                        spacing: 12
                        Text { text: qsTr("界面"); color: theme.textColor; font.pixelSize: 16; font.bold: true; Layout.fillWidth: true }
                        RowLayout {
                            Layout.fillWidth: true
                            Text { text: qsTr("主题"); color: theme.textColor; font.pixelSize: 13; Layout.fillWidth: true }
                            EButton { text: theme.isDark ? qsTr("浅色") : qsTr("深色"); iconCharacter: theme.isDark ? "\uf185" : "\uf186"; size: "xs"; radius: 8; shadowEnabled: false; Layout.preferredWidth: 100; Layout.preferredHeight: 38; onClicked: { theme.toggleTheme(); root.toast.show(qsTr("主题已切换。")) } }
                        }
                        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: theme.borderColor }
                        Text { text: qsTr("窗口默认 1280x720，最小 1100x680。") ; color: Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 0.68); font.pixelSize: 13; wrapMode: Text.WordWrap; Layout.fillWidth: true }
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
                        Text { text: qsTr("后续模块"); color: theme.textColor; font.pixelSize: 16; font.bold: true; Layout.fillWidth: true }
                        EDataTable {
                            headers: [{ key: "name", label: qsTr("模块") }, { key: "status", label: qsTr("状态") }, { key: "note", label: qsTr("说明") }]
                            model: ListModel {
                                ListElement { name: "设备监控"; status: "预留"; note: "连接状态、在线信息、运行数据" }
                                ListElement { name: "系统维护"; status: "预留"; note: "诊断、复位、维护命令" }
                            }
                            selectable: false
                            radius: 8
                            rowHeight: 38
                            headerHeight: 42
                            shadowEnabled: false
                            Layout.fillWidth: true
                            Layout.preferredHeight: 152
                        }
                    }
                }
            }
        }
    }
}
