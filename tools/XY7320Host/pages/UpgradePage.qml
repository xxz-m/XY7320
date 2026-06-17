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

    readonly property bool wideLayout: width >= 980
    readonly property bool manualWideLayout: width >= 1180
    readonly property int formLabelWidth: 64
    readonly property int delayLabelWidth: 78

    readonly property var baudRates: [
        { text: "9600" },
        { text: "57600" },
        { text: "115200" },
        { text: "230400" },
        { text: "460800" },
        { text: "921600" }
    ]

    readonly property var packetSizes: [
        { text: "128 B", value: 128 },
        { text: "256 B", value: 256 },
        { text: "512 B", value: 512 },
        { text: "1024 B", value: 1024 }
    ]

    readonly property var packetDelays: [
        { text: "1 ms", value: 1 },
        { text: "5 ms", value: 5 },
        { text: "10 ms", value: 10 },
        { text: "30 ms", value: 30 }
    ]

    function portModel() {
        var items = []
        for (var i = 0; i < firmwareUploader.ports.length; ++i) {
            items.push({
                text: firmwareUploader.ports[i].text,
                portName: firmwareUploader.ports[i].portName
            })
        }
        return items
    }

    function selectCurrentPort() {
        for (var i = 0; i < firmwareUploader.ports.length; ++i) {
            if (firmwareUploader.ports[i].portName === firmwareUploader.portName) {
                return i
            }
        }
        return firmwareUploader.ports.length > 0 ? 0 : -1
    }

    function selectCurrentBaudRate() {
        var target = String(firmwareUploader.baudRate)
        for (var i = 0; i < baudRates.length; ++i) {
            if (baudRates[i].text === target) {
                return i
            }
        }
        return 2
    }

    function selectCurrentPacketSize() {
        for (var i = 0; i < packetSizes.length; ++i) {
            if (packetSizes[i].value === firmwareUploader.packetSize) {
                return i
            }
        }
        return 3
    }

    function selectCurrentPacketDelay() {
        for (var i = 0; i < packetDelays.length; ++i) {
            if (packetDelays[i].value === firmwareUploader.packetDelayMs) {
                return i
            }
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
            width: parent.width
            spacing: 14

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 12
            }

            Text {
                text: qsTr("固件升级")
                color: theme.textColor
                font.pixelSize: 22
                font.bold: true
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                Layout.fillWidth: true
                elide: Text.ElideRight
            }

            Text {
                text: qsTr("通过 Bootloader USART2 接收口发送链接地址为 0x08010000 的 APP bin。")
                color: Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 0.68)
                font.pixelSize: 13
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                Layout.fillWidth: true
                elide: Text.ElideRight
            }

            GridLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                Layout.bottomMargin: 20
                columns: root.wideLayout ? 2 : 1
                columnSpacing: 16
                rowSpacing: 16

                ECard {
                    Layout.fillWidth: true
                    Layout.minimumWidth: root.wideLayout ? 620 : 0
                    Layout.alignment: Qt.AlignTop
                    radius: 10
                    padding: 18
                    shadowEnabled: false

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 14

                        Text {
                            text: qsTr("升级操作")
                            color: theme.textColor
                            font.pixelSize: 16
                            font.bold: true
                            Layout.fillWidth: true
                        }

                        ColumnLayout {
                            id: upgradeFormBlock
                            z: (portDropdown.opened || baudDropdown.opened) ? 1000 : 0
                            Layout.fillWidth: true
                            spacing: 10

                            GridLayout {
                                id: serialPortRow
                                z: portDropdown.opened ? 1000 : 0
                                Layout.fillWidth: true
                                columns: 3
                                columnSpacing: 10
                                rowSpacing: 10

                                Text {
                                    text: qsTr("串口")
                                    color: theme.textColor
                                    font.pixelSize: 13
                                    Layout.preferredWidth: root.formLabelWidth
                                    verticalAlignment: Text.AlignVCenter
                                }

                                EDropdown {
                                    id: portDropdown
                                    enabled: !firmwareUploader.busy && !firmwareUploader.serialOpen
                                    model: root.portModel()
                                    selectedIndex: root.selectCurrentPort()
                                    title: qsTr("选择串口")
                                    headerHeight: 40
                                    radius: 8
                                    fontSize: 13
                                    popupMaxHeight: 320
                                    scrollBarAlwaysVisible: true
                                    shadowEnabled: false
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 260
                                    Layout.preferredHeight: 40
                                    onSelectionChanged: function(index, item) {
                                        firmwareUploader.portName = item.portName
                                    }
                                }

                                ESwitchButton {
                                    id: serialSwitch
                                    text: firmwareUploader.serialOpen ? qsTr("已开") : qsTr("打开")
                                    checked: firmwareUploader.serialOpen
                                    autoToggle: false
                                    enabled: !firmwareUploader.busy && firmwareUploader.portName.length > 0
                                    size: "s"
                                    radius: 8
                                    fontSize: 13
                                    shadowEnabled: false
                                    Layout.preferredWidth: 112
                                    Layout.preferredHeight: 40
                                    onToggled: function(nextChecked) {
                                        if (nextChecked) {
                                            firmwareUploader.openPort()
                                            if (firmwareUploader.serialOpen) {
                                                toast.show(qsTr("串口已打开。"))
                                            }
                                        } else {
                                            firmwareUploader.closePort()
                                            toast.show(qsTr("串口已关闭。"))
                                        }
                                    }
                                }
                            }

                            GridLayout {
                                id: baudRateRow
                                z: baudDropdown.opened ? 900 : 0
                                Layout.fillWidth: true
                                columns: 2
                                columnSpacing: 10
                                rowSpacing: 10

                                Text {
                                    text: qsTr("波特率")
                                    color: theme.textColor
                                    font.pixelSize: 13
                                    Layout.preferredWidth: root.formLabelWidth
                                    verticalAlignment: Text.AlignVCenter
                                }

                                EDropdown {
                                    id: baudDropdown
                                    enabled: !firmwareUploader.busy && !firmwareUploader.serialOpen
                                    model: root.baudRates
                                    selectedIndex: root.selectCurrentBaudRate()
                                    title: qsTr("115200")
                                    headerHeight: 40
                                    radius: 8
                                    fontSize: 13
                                    popupMaxHeight: 196
                                    shadowEnabled: false
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 136
                                    Layout.preferredHeight: 40
                                    onSelectionChanged: function(index, item) {
                                        firmwareUploader.baudRate = Number(item.text)
                                    }
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 10

                            Text {
                                text: qsTr("固件")
                                color: theme.textColor
                                font.pixelSize: 13
                                Layout.preferredWidth: root.formLabelWidth
                            }

                            EInput {
                                text: firmwareUploader.filePath
                                readOnly: true
                                placeholderText: qsTr("选择链接到 0x08010000 的 APP .bin")
                                radius: 8
                                fontSize: 13
                                shadowEnabled: false
                                enabled: !firmwareUploader.busy
                                Layout.fillWidth: true
                                Layout.preferredHeight: 40
                            }

                            EButton {
                                text: qsTr("浏览")
                                iconCharacter: "\uf07c"
                                size: "xs"
                                radius: 8
                                shadowEnabled: false
                                enabled: !firmwareUploader.busy
                                Layout.preferredWidth: 104
                                Layout.preferredHeight: 40
                                onClicked: root.firmwareDialog.open()
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 10

                            Text {
                                text: qsTr("版本")
                                color: theme.textColor
                                font.pixelSize: 13
                                Layout.preferredWidth: root.formLabelWidth
                            }

                            EInput {
                                text: firmwareUploader.versionText
                                placeholderText: qsTr("yyyyMMddHHmm")
                                radius: 8
                                fontSize: 13
                                shadowEnabled: false
                                enabled: !firmwareUploader.busy
                                Layout.fillWidth: true
                                Layout.preferredHeight: 40
                                onAccepted: firmwareUploader.versionText = text
                            }

                            EInput {
                                text: "0x" + ("0" + Number(firmwareUploader.versionFlag).toString(16).toUpperCase()).slice(-2)
                                readOnly: true
                                radius: 8
                                fontSize: 13
                                shadowEnabled: false
                                Layout.preferredWidth: 104
                                Layout.preferredHeight: 40
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 10

                            Text {
                                text: qsTr("版本帧")
                                color: theme.textColor
                                font.pixelSize: 13
                                Layout.preferredWidth: root.formLabelWidth
                            }

                            EInput {
                                text: firmwareUploader.versionFrameHex
                                readOnly: true
                                placeholderText: qsTr("选择 bin 后按修改时间生成")
                                radius: 8
                                fontSize: 13
                                shadowEnabled: false
                                Layout.fillWidth: true
                                Layout.preferredHeight: 40
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 10

                            Text {
                                text: qsTr("头包")
                                color: theme.textColor
                                font.pixelSize: 13
                                Layout.preferredWidth: root.formLabelWidth
                            }

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
                                Layout.preferredWidth: 104
                                Layout.preferredHeight: 40
                                onClicked: {
                                    firmwareUploader.copyHeaderToClipboard()
                                    toast.show(qsTr("首帧已复制。"))
                                }
                            }
                        }

                        EProgressBar {
                            value: firmwareUploader.progress
                            label: qsTr("%1%").arg(Math.round(firmwareUploader.progress * 100))
                            Layout.fillWidth: true
                            Layout.preferredHeight: 44
                        }

                        ECard {
                            Layout.fillWidth: true
                            Layout.minimumHeight: 154
                            radius: 8
                            padding: 14
                            shadowEnabled: false
                            cardColor: Qt.rgba(theme.focusColor.r, theme.focusColor.g, theme.focusColor.b, 0.08)

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 10

                                Text {
                                    text: qsTr("手动发送")
                                    color: theme.focusColor
                                    font.pixelSize: 14
                                    font.bold: true
                                    Layout.fillWidth: true
                                }

                                Text {
                                    text: qsTr("先手动发送 12 字节头包，再手动启动固件发送。固件会按当前包大小和包间隔自动连续发送。")
                                    color: Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 0.72)
                                    font.pixelSize: 12
                                    wrapMode: Text.WordWrap
                                    Layout.fillWidth: true
                                }

                                GridLayout {
                                    Layout.fillWidth: true
                                    columns: root.manualWideLayout ? 2 : 1
                                    columnSpacing: 10
                                    rowSpacing: 10

                                    EButton {
                                        text: qsTr("发送版本帧")
                                        iconCharacter: "\uf1d8"
                                        size: "xs"
                                        radius: 8
                                        shadowEnabled: false
                                        enabled: !firmwareUploader.busy
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 132
                                        Layout.preferredHeight: 40
                                        onClicked: firmwareUploader.sendVersionFrameManual()
                                    }

                                    EButton {
                                        text: qsTr("发送头包")
                                        iconCharacter: "\uf35b"
                                        size: "xs"
                                        radius: 8
                                        shadowEnabled: false
                                        enabled: !firmwareUploader.busy
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 132
                                        Layout.preferredHeight: 40
                                        onClicked: firmwareUploader.sendHeaderManual()
                                    }

                                    EButton {
                                        text: qsTr("发送固件")
                                        iconCharacter: "\uf1c6"
                                        size: "xs"
                                        radius: 8
                                        shadowEnabled: false
                                        enabled: !firmwareUploader.busy && firmwareUploader.manualHeaderSent
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 132
                                        Layout.preferredHeight: 40
                                        onClicked: firmwareUploader.sendFirmwareManual()
                                    }
                                }

                                Text {
                                    text: firmwareUploader.manualHeaderSent
                                          ? qsTr("头包已发送，可以继续发送固件。")
                                          : qsTr("头包未发送。")
                                    color: firmwareUploader.manualHeaderSent
                                           ? theme.focusColor
                                           : Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 0.7)
                                    font.pixelSize: 12
                                    wrapMode: Text.WordWrap
                                    Layout.fillWidth: true
                                }
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10

                                EButton {
                                    text: firmwareUploader.busy ? qsTr("升级中") : qsTr("开始升级")
                                    iconCharacter: firmwareUploader.busy ? "\uf110" : "\uf35b"
                                    iconRotateOnClick: false
                                    size: "xs"
                                    radius: 8
                                    containerColor: theme.focusColor
                                    hoverColor: Qt.darker(theme.focusColor, 1.12)
                                    textColor: "white"
                                    iconColor: "white"
                                    enabled: !firmwareUploader.busy
                                    Layout.preferredWidth: 148
                                    Layout.preferredHeight: 40
                                    onClicked: root.requestStart()
                                }

                                EButton {
                                    text: qsTr("取消")
                                    iconCharacter: "\uf00d"
                                    size: "xs"
                                    radius: 8
                                    shadowEnabled: false
                                    enabled: firmwareUploader.busy || firmwareUploader.manualHeaderSent
                                    Layout.preferredWidth: 104
                                    Layout.preferredHeight: 40
                                    onClicked: firmwareUploader.cancel()
                                }

                                EButton {
                                    text: qsTr("清空日志")
                                    iconCharacter: "\uf2ed"
                                    size: "xs"
                                    radius: 8
                                    shadowEnabled: false
                                    enabled: !firmwareUploader.busy
                                    Layout.preferredWidth: 116
                                    Layout.preferredHeight: 40
                                    onClicked: firmwareUploader.clearLog()
                                }

                                Item {
                                    Layout.fillWidth: true
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 28
                                spacing: 8

                                Item {
                                    Layout.fillWidth: true
                                }

                                ELoader {
                                    visible: firmwareUploader.busy
                                    running: firmwareUploader.busy
                                    size: 24
                                    Layout.preferredWidth: 28
                                    Layout.preferredHeight: 28
                                }

                                Text {
                                    text: firmwareUploader.status
                                    color: firmwareUploader.busy ? theme.focusColor : theme.textColor
                                    font.pixelSize: 13
                                    font.bold: firmwareUploader.busy
                                    horizontalAlignment: Text.AlignRight
                                    verticalAlignment: Text.AlignVCenter
                                    Layout.minimumWidth: 120
                                    Layout.preferredWidth: 180
                                    elide: Text.ElideRight
                                }
                            }
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: !root.wideLayout
                    Layout.preferredWidth: root.wideLayout ? 340 : 0
                    Layout.minimumWidth: root.wideLayout ? 300 : 0
                    Layout.alignment: Qt.AlignTop
                    spacing: 16

                    ECard {
                        Layout.fillWidth: true
                        radius: 10
                        padding: 16
                        shadowEnabled: false

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            Text {
                                text: qsTr("固件信息")
                                color: theme.textColor
                                font.pixelSize: 15
                                font.bold: true
                                Layout.fillWidth: true
                            }

                            InfoRow { label: qsTr("文件"); value: firmwareUploader.fileName }
                            InfoRow { label: qsTr("大小"); value: firmwareUploader.fileSizeText }
                            InfoRow { label: qsTr("CRC32"); value: firmwareUploader.crcHex }
                        }
                    }

                    ECard {
                        Layout.fillWidth: true
                        radius: 10
                        padding: 16
                        shadowEnabled: false

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            Text {
                                text: qsTr("协议参数")
                                color: theme.textColor
                                font.pixelSize: 15
                                font.bold: true
                                Layout.fillWidth: true
                            }

                            InfoRow { label: qsTr("Magic"); value: "0x41505055" }
                            InfoRow { label: qsTr("APP地址"); value: "0x08010000" }
                            InfoRow { label: qsTr("最大包"); value: "1024 B" }
                            InfoRow { label: qsTr("串口"); value: "USART2 / 8N1" }
                        }
                    }

                    ECard {
                        Layout.fillWidth: true
                        radius: 10
                        padding: 16
                        shadowEnabled: false

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            Text {
                                text: qsTr("发送时序")
                                color: theme.textColor
                                font.pixelSize: 15
                                font.bold: true
                                Layout.fillWidth: true
                            }

                            DelayRow {
                                label: qsTr("擦除等待")
                                value: firmwareUploader.headerDelayMs
                                suffix: " ms"
                                enabled: !firmwareUploader.busy
                                onChanged: function(v) { firmwareUploader.headerDelayMs = v }
                            }

                        }
                    }

                    ECard {
                        id: packetSettingsCard
                        z: (packetSizeDropdown.opened || packetDelayDropdown.opened) ? 1000 : 0
                        Layout.fillWidth: true
                        radius: 10
                        padding: 16
                        shadowEnabled: false

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            Text {
                                text: qsTr("发包设置")
                                color: theme.textColor
                                font.pixelSize: 15
                                font.bold: true
                                Layout.fillWidth: true
                            }

                            RowLayout {
                                z: packetSizeDropdown.opened ? 1000 : 0
                                Layout.fillWidth: true
                                spacing: 10

                                Text {
                                    text: qsTr("包大小")
                                    color: Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 0.58)
                                    font.pixelSize: 12
                                    Layout.preferredWidth: root.delayLabelWidth
                                    verticalAlignment: Text.AlignVCenter
                                    elide: Text.ElideRight
                                }

                                EDropdown {
                                    id: packetSizeDropdown
                                    enabled: !firmwareUploader.busy
                                    model: root.packetSizes
                                    selectedIndex: root.selectCurrentPacketSize()
                                    title: qsTr("1024 B")
                                    headerHeight: 36
                                    radius: 8
                                    fontSize: 12
                                    popupMaxHeight: 220
                                    popupDirection: 1
                                    scrollBarAlwaysVisible: true
                                    shadowEnabled: false
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 150
                                    Layout.preferredHeight: 36
                                    onSelectionChanged: function(index, item) {
                                        firmwareUploader.packetSize = item.value
                                    }
                                }
                            }

                            RowLayout {
                                z: packetDelayDropdown.opened ? 900 : 0
                                Layout.fillWidth: true
                                spacing: 10

                                Text {
                                    text: qsTr("间隔")
                                    color: Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 0.58)
                                    font.pixelSize: 12
                                    Layout.preferredWidth: root.delayLabelWidth
                                    verticalAlignment: Text.AlignVCenter
                                    elide: Text.ElideRight
                                }

                                EDropdown {
                                    id: packetDelayDropdown
                                    enabled: !firmwareUploader.busy
                                    model: root.packetDelays
                                    selectedIndex: root.selectCurrentPacketDelay()
                                    title: qsTr("10 ms")
                                    headerHeight: 36
                                    radius: 8
                                    fontSize: 12
                                    popupMaxHeight: 220
                                    popupDirection: 1
                                    scrollBarAlwaysVisible: true
                                    shadowEnabled: false
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 150
                                    Layout.preferredHeight: 36
                                    onSelectionChanged: function(index, item) {
                                        firmwareUploader.packetDelayMs = item.value
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    component InfoRow: RowLayout {
        property string label: ""
        property string value: ""

        Layout.fillWidth: true
        spacing: 10

        Text {
            text: label
            color: Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 0.58)
            font.pixelSize: 12
            Layout.preferredWidth: 68
            elide: Text.ElideRight
        }

        Text {
            text: value
            color: theme.textColor
            font.pixelSize: 12
            horizontalAlignment: Text.AlignRight
            Layout.fillWidth: true
            elide: Text.ElideRight
        }
    }

    component DelayRow: RowLayout {
        property string label: ""
        property int value: 0
        property string suffix: ""
        signal changed(int value)

        Layout.fillWidth: true
        spacing: 10

        Text {
            text: label
            color: Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 0.58)
            font.pixelSize: 12
            Layout.preferredWidth: root.delayLabelWidth
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        SpinBox {
            from: label === qsTr("擦除等待") ? 0 : 1
            to: label === qsTr("擦除等待") ? 30000 : 5000
            value: parent.value
            editable: true
            enabled: parent.enabled
            Layout.fillWidth: true
            Layout.minimumWidth: 190
            Layout.preferredHeight: 36
            onValueModified: parent.changed(value)
            textFromValue: function(value) { return value + parent.suffix }
            valueFromText: function(text) { return parseInt(text) }
        }
    }
}
