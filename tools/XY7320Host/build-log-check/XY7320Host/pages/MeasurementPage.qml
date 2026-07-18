import QtQuick
import QtQuick.Layouts
import XY7320Host

Item {
    id: root

    required property string modeName
    required property bool dmrMode
    property string unit: "W"
    property bool paused: false
    property bool locked: false
    property bool pulseEnabled: false
    signal requestReturn()

    readonly property color panelColor: theme.instrumentPanelColor
    readonly property color insetColor: theme.instrumentInsetColor
    readonly property color borderColor: theme.instrumentBorderColor
    readonly property color activeColor: theme.instrumentActiveColor

    readonly property var gsmMeters: [
        { label: qsTr("GSM 正向功率 (%1)").arg(unit), value: unit === "W" ? "42.80" : "46.31" },
        { label: qsTr("GSM 驻波比"), value: "1.18" }
    ]
    readonly property var dmrMeters: [
        { label: qsTr("413 功率 (%1)").arg(unit), value: unit === "W" ? "38.60" : "45.87" },
        { label: qsTr("413 驻波比"), value: "1.12" },
        { label: qsTr("457 功率 (%1)").arg(unit), value: unit === "W" ? "41.20" : "46.15" },
        { label: qsTr("457 驻波比"), value: "1.16" }
    ]
    readonly property var meters: dmrMode ? dmrMeters : gsmMeters

    RowLayout {
        anchors.fill: parent
        anchors.margins: 22
        spacing: 12

        Rectangle {
            id: meterPanel
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 14
            color: root.panelColor
            border.width: 2
            border.color: root.borderColor

            GridLayout {
                anchors.fill: parent
                anchors.margins: root.dmrMode ? 34 : 48
                columns: root.dmrMode ? 2 : 2
                rows: root.dmrMode ? 2 : 1
                columnSpacing: root.dmrMode ? 38 : 34
                rowSpacing: 28

                Repeater {
                    model: root.meters

                    delegate: ColumnLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 12

                        Text {
                            text: modelData.label
                            color: theme.textColor
                            font.pixelSize: root.dmrMode ? 18 : 20
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            Layout.fillWidth: true
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            Layout.minimumHeight: root.dmrMode ? 150 : 210
                            radius: 12
                            color: root.insetColor
                            border.width: 3
                            border.color: root.borderColor

                            Rectangle {
                                anchors.fill: parent
                                anchors.margins: 5
                                radius: 8
                                color: "transparent"
                                border.width: 1
                                border.color: Qt.rgba(root.borderColor.r, root.borderColor.g, root.borderColor.b, 0.65)
                            }

                            Text {
                                anchors.centerIn: parent
                                text: modelData.value
                                color: "#071425"
                                font.family: "Consolas"
                                font.pixelSize: root.dmrMode ? 74 : 106
                                font.letterSpacing: 3
                                font.bold: true
                            }
                        }
                    }
                }
            }
        }

        Rectangle {
            Layout.preferredWidth: 166
            Layout.fillHeight: true
            radius: 14
            color: root.panelColor
            border.width: 2
            border.color: root.borderColor

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 18
                spacing: 12

                Text {
                    text: qsTr("单位")
                    color: theme.textColor
                    font.pixelSize: 16
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    Layout.fillWidth: true
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 48
                    radius: 8
                    color: root.insetColor
                    border.width: 2
                    border.color: root.borderColor

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 2
                        spacing: 0

                        Repeater {
                            model: ["W", "dBm"]
                            delegate: Rectangle {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                radius: 6
                                color: root.unit === modelData ? root.activeColor : "transparent"

                                Text {
                                    anchors.centerIn: parent
                                    text: modelData
                                    color: root.unit === modelData ? "white" : theme.textColor
                                    font.pixelSize: 16
                                    font.bold: true
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: root.unit = modelData
                                }
                            }
                        }
                    }
                }

                InstrumentButton {
                    text: root.paused ? qsTr("启动") : qsTr("暂停")
                    icon: root.paused ? "\uf04b" : "\uf04c"
                    active: !root.paused
                    onClicked: root.paused = !root.paused
                }

                InstrumentButton {
                    text: qsTr("锁存")
                    icon: "\uf023"
                    active: root.locked
                    onClicked: root.locked = !root.locked
                }

                InstrumentButton {
                    text: qsTr("脉宽")
                    icon: "\uf1e6"
                    active: root.pulseEnabled
                    onClicked: root.pulseEnabled = !root.pulseEnabled
                }

                InstrumentButton {
                    text: qsTr("返回")
                    icon: "\uf060"
                    onClicked: root.requestReturn()
                }

                Item { Layout.fillHeight: true }
            }
        }
    }

    component InstrumentButton: Rectangle {
        required property string text
        required property string icon
        property bool active: false
        signal clicked()

        Layout.fillWidth: true
        Layout.preferredHeight: 86
        radius: 10
        color: active ? root.activeColor : root.insetColor
        border.width: 2
        border.color: root.borderColor

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 8
            spacing: 2

            Text {
                text: parent.parent.icon
                font.family: iconFont.name
                font.pixelSize: 25
                color: parent.parent.active ? "white" : theme.textColor
                horizontalAlignment: Text.AlignHCenter
                Layout.fillWidth: true
            }

            Text {
                text: parent.parent.text
                color: parent.parent.active ? "white" : theme.textColor
                font.pixelSize: 16
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                Layout.fillWidth: true
            }
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: parent.clicked()
        }
    }
}
