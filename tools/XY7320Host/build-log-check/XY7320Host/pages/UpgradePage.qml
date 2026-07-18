import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic
import QtQuick.Layouts
import XY7320Host

Item {
    id: root

    required property var firmwareDialog
    required property var startDialog
    required property var toast

    readonly property bool wideLayout: width >= 900
    readonly property color mutedText: Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 0.62)
    readonly property color subtleText: Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 0.46)
    readonly property int labelWidth: 82
    readonly property var baudRates: [
        { text: "9600" },
        { text: "57600" },
        { text: "115200" },
        { text: "230400" },
        { text: "460800" },
        { text: "921600" }
    ]

    function portModel() {
        var items = []
        for (var i = 0; i < firmwareUploader.ports.length; ++i) {
            items.push({ text: firmwareUploader.ports[i].text, portName: firmwareUploader.ports[i].portName })
        }
        return items
    }

    function currentPort() {
        for (var i = 0; i < firmwareUploader.ports.length; ++i) {
            if (firmwareUploader.ports[i].portName === firmwareUploader.portName) return i
        }
        return firmwareUploader.ports.length > 0 ? 0 : -1
    }

    function currentBaud() {
        for (var i = 0; i < baudRates.length; ++i) {
            if (baudRates[i].text === String(firmwareUploader.baudRate)) return i
        }
        return 2
    }

    function requestStart() {
        if (firmwareUploader.portName.length === 0) {
            toast.show(qsTr("请先选择串口。"))
            return
        }
        if (firmwareUploader.filePath.length === 0) {
            toast.show(qsTr("请先选择 APP bin 固件。"))
            return
        }
        startDialog.open()
    }

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth
        clip: true

        ColumnLayout {
            width: Math.max(parent.width, 1120)
            spacing: 16

            Item { Layout.preferredHeight: 10 }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 28
                Layout.rightMargin: 28
                spacing: 16

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    Text {
                        text: qsTr("固件升级")
                        color: theme.textColor
                        font.pixelSize: 25
                        font.bold: true
                    }
                    Text {
                        text: qsTr("将 APP 固件安全发送到设备")
                        color: root.mutedText
                        font.pixelSize: 13
                    }
                }

                EButton {
                    text: qsTr("刷新串口")
                    iconCharacter: "\uf021"
                    size: "xs"
                    radius: 8
                    shadowEnabled: false
                    enabled: !firmwareUploader.busy
                    Layout.preferredWidth: 112
                    Layout.preferredHeight: 38
                    onClicked: firmwareUploader.refreshPorts()
                }
            }

            ECard {
                Layout.fillWidth: true
                Layout.leftMargin: 28
                Layout.rightMargin: 28
                padding: 18
                radius: 10
                shadowEnabled: false
                cardColor: theme.instrumentPanelColor
                borderColor: theme.instrumentBorderColor
                borderWidth: 1

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    StepItem { title: qsTr("选择固件"); number: "1"; active: firmwareUploader.filePath.length > 0; complete: firmwareUploader.filePath.length > 0; Layout.fillWidth: true }
                    StepLine { Layout.fillWidth: true }
                    StepItem { title: qsTr("设备连接"); number: "2"; active: firmwareUploader.serialOpen; complete: firmwareUploader.serialOpen; Layout.fillWidth: true }
                    StepLine { Layout.fillWidth: true }
                    StepItem { title: qsTr("开始升级"); number: "3"; active: firmwareUploader.busy || firmwareUploader.progress > 0; complete: firmwareUploader.progress >= 1; Layout.fillWidth: true }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 28
                Layout.rightMargin: 28
                Layout.bottomMargin: 28
                spacing: 16

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 910
                    Layout.minimumWidth: 700
                    Layout.alignment: Qt.AlignTop
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

                            SectionTitle { text: qsTr("选择固件") }

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
                                            text: firmwareUploader.fileName.length > 0 ? firmwareUploader.fileName : qsTr("尚未选择固件")
                                            color: theme.textColor
                                            font.pixelSize: 15
                                            font.bold: true
                                            Layout.fillWidth: true
                                            elide: Text.ElideRight
                                        }
                                        Text {
                                            text: firmwareUploader.filePath.length > 0 ? qsTr("固件已载入，可以开始检查设备连接") : qsTr("请选择 APP .bin 固件文件")
                                            color: root.mutedText
                                            font.pixelSize: 12
                                            Layout.fillWidth: true
                                            elide: Text.ElideRight
                                        }
                                    }

                                    EButton {
                                        text: qsTr("浏览")
                                        iconCharacter: "\uf07c"
                                        size: "xs"
                                        radius: 8
                                        shadowEnabled: false
                                        enabled: !firmwareUploader.busy
                                        Layout.preferredWidth: 100
                                        Layout.preferredHeight: 40
                                        onClicked: root.firmwareDialog.open()
                                    }
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 12
                                InfoBox { label: qsTr("版本"); value: firmwareUploader.versionText.length > 0 ? firmwareUploader.versionText : qsTr("未设置"); Layout.fillWidth: true }
                                InfoBox { label: qsTr("文件大小"); value: firmwareUploader.fileSizeText; Layout.fillWidth: true }
                            }
                        }
                    }

                    ECard {
                        id: connectionCard
                        z: (portDropdown.opened || baudDropdown.opened) ? 10 : 0
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
                            SectionTitle { text: qsTr("设备连接") }
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 12
                                Text { text: qsTr("串口"); color: root.mutedText; font.pixelSize: 12; Layout.preferredWidth: root.labelWidth }
                                EDropdown {
                                    id: portDropdown
                                    enabled: !firmwareUploader.busy && !firmwareUploader.serialOpen
                                    model: root.portModel()
                                    selectedIndex: root.currentPort()
                                    title: qsTr("选择串口")
                                    headerHeight: 40
                                    radius: 8
                                    fontSize: 13
                                    popupMaxHeight: 300
                                    scrollBarAlwaysVisible: true
                                    shadowEnabled: false
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 40
                                    onSelectionChanged: function(index, item) { firmwareUploader.portName = item.portName }
                                }
                                Text { text: firmwareUploader.serialOpen ? qsTr("已连接") : qsTr("未连接"); color: firmwareUploader.serialOpen ? theme.focusColor : root.mutedText; font.pixelSize: 13; font.bold: firmwareUploader.serialOpen; Layout.preferredWidth: 64; horizontalAlignment: Text.AlignRight }
                            }
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 12
                                Text { text: qsTr("波特率"); color: root.mutedText; font.pixelSize: 12; Layout.preferredWidth: root.labelWidth }
                                EDropdown {
                                    id: baudDropdown
                                    enabled: !firmwareUploader.busy && !firmwareUploader.serialOpen
                                    model: root.baudRates
                                    selectedIndex: root.currentBaud()
                                    title: qsTr("115200")
                                    headerHeight: 40
                                    radius: 8
                                    fontSize: 13
                                    popupMaxHeight: 220
                                    shadowEnabled: false
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 40
                                    onSelectionChanged: function(index, item) { firmwareUploader.baudRate = Number(item.text) }
                                }
                                ESwitchButton {
                                    text: firmwareUploader.serialOpen ? qsTr("已连接") : qsTr("打开串口")
                                    checked: firmwareUploader.serialOpen
                                    autoToggle: false
                                    enabled: !firmwareUploader.busy && firmwareUploader.portName.length > 0
                                    size: "s"
                                    radius: 8
                                    fontSize: 13
                                    shadowEnabled: false
                                    Layout.preferredWidth: 128
                                    Layout.preferredHeight: 40
                                    onToggled: function(nextChecked) {
                                        if (nextChecked) firmwareUploader.openPort()
                                        else firmwareUploader.closePort()
                                    }
                                }
                            }
                        }
                    }

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
                            SectionTitle { text: qsTr("升级准备") }
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 12
                                Text { text: qsTr("头包（12字节）"); color: root.mutedText; font.pixelSize: 12; Layout.preferredWidth: 104 }
                                EInput {
                                    text: firmwareUploader.headerHex
                                    readOnly: true
                                    placeholderText: qsTr("选择固件后自动生成 12 字节头包")
                                    radius: 8
                                    fontSize: 13
                                    shadowEnabled: false
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 40
                                }
                                EButton {
                                    text: qsTr("复制")
                                    iconCharacter: "\uf0c5"
                                    size: "xs"
                                    radius: 8
                                    shadowEnabled: false
                                    enabled: firmwareUploader.headerHex.length > 0
                                    Layout.preferredWidth: 92
                                    Layout.preferredHeight: 40
                                    onClicked: { firmwareUploader.copyHeaderToClipboard(); toast.show(qsTr("头包已复制。")) }
                                }
                            }
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 12
                                EButton {
                                    text: firmwareUploader.busy ? qsTr("升级中") : qsTr("开始升级")
                                    iconCharacter: firmwareUploader.busy ? "\uf110" : "\uf35b"
                                    size: "s"
                                    radius: 8
                                    containerColor: theme.focusColor
                                    hoverColor: Qt.darker(theme.focusColor, 1.12)
                                    textColor: "white"
                                    iconColor: "white"
                                    enabled: !firmwareUploader.busy
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 48
                                    onClicked: root.requestStart()
                                }
                            }
                        }
                    }

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
                            spacing: 14
                            RowLayout {
                                Layout.fillWidth: true
                                SectionTitle { text: qsTr("升级进度"); Layout.fillWidth: true }
                                ELoader { visible: firmwareUploader.busy; running: firmwareUploader.busy; size: 20; Layout.preferredWidth: 22; Layout.preferredHeight: 22 }
                                Text { text: qsTr("%1%").arg(Math.round(firmwareUploader.progress * 100)); color: theme.focusColor; font.pixelSize: 15; font.bold: true }
                                EButton {
                                    text: qsTr("取消")
                                    iconCharacter: "\uf00d"
                                    size: "xs"
                                    radius: 8
                                    shadowEnabled: false
                                    visible: firmwareUploader.busy
                                    enabled: firmwareUploader.busy
                                    Layout.preferredWidth: 88
                                    Layout.preferredHeight: 34
                                    onClicked: firmwareUploader.cancel()
                                }
                            }
                            EProgressBar {
                                value: firmwareUploader.progress
                                label: ""
                                showHeader: false
                                Layout.fillWidth: true
                                Layout.preferredHeight: 12
                            }
                            Text {
                                text: firmwareUploader.status
                                color: root.mutedText
                                font.pixelSize: 12
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                            }
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 56
                                radius: 8
                                color: Qt.rgba(theme.focusColor.r, theme.focusColor.g, theme.focusColor.b, 0.08)
                                border.color: Qt.rgba(theme.focusColor.r, theme.focusColor.g, theme.focusColor.b, 0.28)
                                border.width: 1
                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 12
                                    spacing: 10
                                    Text { text: "\uf023"; font.family: "Font Awesome 6 Free"; font.pixelSize: 17; color: theme.focusColor; Layout.preferredWidth: 22 }
                                    Text { text: qsTr("安全提示：升级过程中请保持串口连接稳定，不要关闭设备电源。"); color: root.mutedText; font.pixelSize: 12; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                                }
                            }
                        }
                    }
                }

                ColumnLayout {
                    Layout.preferredWidth: 410
                    Layout.minimumWidth: 360
                    Layout.alignment: Qt.AlignTop
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
                            spacing: 12
                            SectionTitle { text: qsTr("固件信息") }
                            InfoRow { label: qsTr("文件名"); value: firmwareUploader.fileName }
                            Divider {}
                            InfoRow { label: qsTr("文件大小"); value: firmwareUploader.fileSizeText }
                            Divider {}
                            InfoRow { label: qsTr("CRC32"); value: firmwareUploader.crcHex }
                        }
                    }

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
                            spacing: 12
                            SectionTitle { text: qsTr("协议参数") }
                            InfoRow { label: qsTr("Magic"); value: "0x41505055" }
                            InfoRow { label: qsTr("APP地址"); value: "0x08010000" }
                            InfoRow { label: qsTr("最大包"); value: "1024 B" }
                            InfoRow { label: qsTr("串口"); value: "USART2 / 8N1" }
                        }
                    }

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
                            spacing: 12
                            SectionTitle { text: qsTr("发送设置") }
                            InfoRow { label: qsTr("擦除等待"); value: firmwareUploader.headerDelayMs + " ms" }
                            InfoRow { label: qsTr("包大小"); value: firmwareUploader.packetSize + " B" }
                            InfoRow { label: qsTr("间隔"); value: firmwareUploader.packetDelayMs + " ms" }
                        }
                    }
                }
            }
        }
    }

    component SectionTitle: Text {
        color: theme.textColor
        font.pixelSize: 15
        font.bold: true
        Layout.fillWidth: true
    }

    component StepItem: ColumnLayout {
        id: stepItem
        property string title: ""
        property string number: ""
        property bool active: false
        property bool complete: false
        spacing: 6
        Layout.alignment: Qt.AlignVCenter
        Rectangle {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: 28
            Layout.preferredHeight: 28
            radius: 14
            color: stepItem.complete ? theme.focusColor : (stepItem.active ? Qt.rgba(theme.focusColor.r, theme.focusColor.g, theme.focusColor.b, 0.18) : theme.secondaryColor)
            border.color: stepItem.active || stepItem.complete ? theme.focusColor : theme.instrumentBorderColor
            border.width: 1
            Text {
                anchors.centerIn: parent
                text: stepItem.complete ? "\uf00c" : stepItem.number
                color: stepItem.complete ? "white" : (stepItem.active ? theme.focusColor : root.mutedText)
                font.family: stepItem.complete ? "Font Awesome 6 Free" : "Arial"
                font.pixelSize: stepItem.complete ? 12 : 13
                font.bold: true
            }
        }
        Text {
            text: stepItem.title
            color: stepItem.active || stepItem.complete ? theme.textColor : root.mutedText
            font.pixelSize: 12
            font.bold: stepItem.active || stepItem.complete
            horizontalAlignment: Text.AlignHCenter
            Layout.fillWidth: true
        }
    }

    component StepLine: Rectangle {
        Layout.preferredHeight: 1
        Layout.alignment: Qt.AlignVCenter
        color: theme.instrumentBorderColor
        Layout.minimumWidth: 20
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

    component InfoRow: RowLayout {
        id: infoRow
        property string label: ""
        property string value: ""
        Layout.fillWidth: true
        spacing: 10
        Text { text: infoRow.label; color: root.mutedText; font.pixelSize: 12; Layout.fillWidth: true; elide: Text.ElideRight }
        Text { text: infoRow.value; color: theme.textColor; font.pixelSize: 12; horizontalAlignment: Text.AlignRight; Layout.preferredWidth: 190; elide: Text.ElideLeft }
    }

    component Divider: Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: 1
        color: theme.borderColor
    }

    Connections {
        target: firmwareUploader
        function onSerialOpenChanged() {
            if (firmwareUploader.serialOpen) toast.show(qsTr("串口已打开。"))
        }
    }
}
