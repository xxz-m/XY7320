import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

ApplicationWindow {
    id: window
    width: 860
    height: 620
    visible: true
    title: "XY7320 Bootloader 升级工具 Demo"

    FileDialog {
        id: firmwareDialog
        title: "选择 APP 固件 bin"
        nameFilters: ["固件文件 (*.bin)", "所有文件 (*)"]
        onAccepted: firmwareUploader.setFileUrl(selectedFile)
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 18
        spacing: 14

        Label {
            text: "XY7320 Bootloader 串口升级"
            font.pixelSize: 24
            font.bold: true
            Layout.fillWidth: true
        }

        GridLayout {
            columns: 4
            columnSpacing: 12
            rowSpacing: 10
            Layout.fillWidth: true

            Label { text: "串口" }
            ComboBox {
                id: portBox
                model: firmwareUploader.ports
                enabled: !firmwareUploader.busy
                Layout.fillWidth: true
                onActivated: firmwareUploader.portName = currentText
                Component.onCompleted: currentIndex = Math.max(0, indexOfValue(firmwareUploader.portName))
            }

            Button {
                text: "刷新"
                enabled: !firmwareUploader.busy
                onClicked: firmwareUploader.refreshPorts()
            }

            Label { text: firmwareUploader.portName }

            Label { text: "波特率" }
            ComboBox {
                model: [9600, 57600, 115200, 230400, 460800, 921600]
                enabled: !firmwareUploader.busy
                editable: true
                currentIndex: 2
                Layout.fillWidth: true
                onAccepted: firmwareUploader.baudRate = parseInt(editText)
                onActivated: firmwareUploader.baudRate = parseInt(currentText)
            }

            Label { text: "包大小" }
            SpinBox {
                from: 1
                to: 1024
                value: firmwareUploader.packetSize
                enabled: !firmwareUploader.busy
                editable: true
                textFromValue: function(value) { return value + " B" }
                valueFromText: function(text) { return parseInt(text) }
                onValueModified: firmwareUploader.packetSize = value
            }

            Label { text: "包间隔" }
            SpinBox {
                from: 1
                to: 5000
                value: firmwareUploader.packetDelayMs
                enabled: !firmwareUploader.busy
                editable: true
                textFromValue: function(value) { return value + " ms" }
                valueFromText: function(text) { return parseInt(text) }
                onValueModified: firmwareUploader.packetDelayMs = value
            }

            Label { text: "擦除等待" }
            SpinBox {
                from: 0
                to: 30000
                value: firmwareUploader.headerDelayMs
                enabled: !firmwareUploader.busy
                editable: true
                textFromValue: function(value) { return value + " ms" }
                valueFromText: function(text) { return parseInt(text) }
                onValueModified: firmwareUploader.headerDelayMs = value
            }

            Label { text: "固件" }
            TextField {
                text: firmwareUploader.filePath
                readOnly: true
                Layout.columnSpan: 2
                Layout.fillWidth: true
                placeholderText: "选择链接地址为 0x08010000 的 APP .bin"
            }

            Button {
                text: "浏览"
                enabled: !firmwareUploader.busy
                onClicked: firmwareDialog.open()
            }

            Label { text: "头包HEX" }
            TextField {
                text: firmwareUploader.headerHex
                readOnly: true
                selectByMouse: true
                Layout.columnSpan: 3
                Layout.fillWidth: true
                placeholderText: "选择固件后自动计算 12 字节头包"
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Button {
                text: firmwareUploader.busy ? "发送中..." : "开始升级"
                enabled: !firmwareUploader.busy
                Layout.preferredWidth: 150
                onClicked: firmwareUploader.start()
            }

            Button {
                text: "取消"
                enabled: firmwareUploader.busy
                onClicked: firmwareUploader.cancel()
            }

            Button {
                text: "清空日志"
                enabled: !firmwareUploader.busy
                onClicked: firmwareUploader.clearLog()
            }

            Label {
                text: firmwareUploader.status
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignRight
            }
        }

        ProgressBar {
            from: 0
            to: 1
            value: firmwareUploader.progress
            Layout.fillWidth: true
        }

        TextArea {
            text: firmwareUploader.logText
            readOnly: true
            wrapMode: TextArea.NoWrap
            font.family: "Consolas"
            font.pixelSize: 13
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        Label {
            text: "协议：先发送小端头 {0x41505055, app_size, crc32}，再发送 APP 原始数据。当前 bootloader 的 USART2 DMA 缓冲为 1024 字节，所以单包最大 1024 字节。"
            color: "#60646c"
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
    }
}
