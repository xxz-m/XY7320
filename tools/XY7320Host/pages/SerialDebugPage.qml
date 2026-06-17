import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic
import QtQuick.Layouts
import XY7320Host

Item {
    id: root

    readonly property bool wideLayout: width >= 980
    readonly property int formLabelWidth: 64

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
        for (var i = 0; i < serialDebug.ports.length; ++i) {
            items.push({
                text: serialDebug.ports[i].text,
                portName: serialDebug.ports[i].portName
            })
        }
        return items
    }

    function selectCurrentPort() {
        for (var i = 0; i < serialDebug.ports.length; ++i) {
            if (serialDebug.ports[i].portName === serialDebug.portName) {
                return i
            }
        }
        return serialDebug.ports.length > 0 ? 0 : -1
    }

    function selectCurrentBaudRate() {
        var target = String(serialDebug.baudRate)
        for (var i = 0; i < baudRates.length; ++i) {
            if (baudRates[i].text === target) {
                return i
            }
        }
        return 2
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
                text: qsTr("串口调试")
                color: theme.textColor
                font.pixelSize: 22
                font.bold: true
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                Layout.fillWidth: true
                elide: Text.ElideRight
            }

            Text {
                text: qsTr("通过 USART1 调试串口收发数据，查看下位机 LOG_Printf 输出。")
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

                // 左侧：串口配置 + 接收区
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.minimumWidth: root.wideLayout ? 620 : 0
                    Layout.alignment: Qt.AlignTop
                    spacing: 14

                    // 串口配置卡片
                    ECard {
                        Layout.fillWidth: true
                        radius: 10
                        padding: 18
                        shadowEnabled: false

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 14

                            Text {
                                text: qsTr("串口配置")
                                color: theme.textColor
                                font.pixelSize: 16
                                font.bold: true
                                Layout.fillWidth: true
                            }

                            GridLayout {
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
                                    enabled: !serialDebug.isOpen
                                    model: serialDebug.ports
                                    selectedIndex: root.selectCurrentPort()
                                    title: qsTr("选择串口")
                                    headerHeight: 40
                                    radius: 8
                                    fontSize: 13
                                    popupMaxHeight: 180
                                    shadowEnabled: false
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 200
                                    Layout.preferredHeight: 40
                                    onSelectionChanged: function(index, item) {
                                        serialDebug.portName = item.portName
                                    }
                                }

                                ESwitchButton {
                                    id: serialSwitch
                                    text: serialDebug.isOpen ? qsTr("已开") : qsTr("打开")
                                    checked: serialDebug.isOpen
                                    autoToggle: false
                                    enabled: serialDebug.portName.length > 0
                                    size: "s"
                                    radius: 8
                                    fontSize: 13
                                    shadowEnabled: false
                                    Layout.preferredWidth: 100
                                    Layout.preferredHeight: 40
                                    onToggled: function(nextChecked) {
                                        if (nextChecked) {
                                            serialDebug.open()
                                        } else {
                                            serialDebug.close()
                                        }
                                    }
                                }
                            }

                            GridLayout {
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
                                    enabled: !serialDebug.isOpen
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
                                        serialDebug.baudRate = Number(item.text)
                                    }
                                }
                            }
                        }
                    }

                    // 接收区卡片
                    ECard {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 400
                        radius: 10
                        padding: 0
                        shadowEnabled: false

                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 0

                            // 工具栏
                            RowLayout {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 40
                                Layout.leftMargin: 14
                                Layout.rightMargin: 14
                                spacing: 10

                                Text {
                                    text: qsTr("接收区")
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

                                ECheckBox {
                                    text: qsTr("HEX")
                                    checked: serialDebug.showHex
                                    onToggled: serialDebug.showHex = checked
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
                            Flickable {
                                id: logFlickable
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                Layout.margins: 8
                                contentWidth: width
                                contentHeight: logText.implicitHeight
                                clip: true
                                boundsBehavior: Flickable.StopAtBounds

                                function scrollToBottom() {
                                    contentY = Math.max(0, contentHeight - height)
                                }

                                onContentHeightChanged: {
                                    if (serialDebug.autoScroll && contentY >= contentHeight - height - 20) {
                                        scrollToBottom()
                                    }
                                }

                                ScrollBar.vertical: ScrollBar {
                                    policy: ScrollBar.AsNeeded
                                    width: 8
                                }

                                TextArea {
                                    id: logText
                                    width: logFlickable.width
                                    height: implicitHeight
                                    text: serialDebug.logText
                                    readOnly: true
                                    wrapMode: TextArea.Wrap
                                    selectByMouse: true
                                    color: theme.textColor
                                    font.family: "Consolas"
                                    font.pixelSize: 12
                                    textMargin: 0
                                    background: Rectangle { color: "transparent" }

                                    onTextChanged: {
                                        if (serialDebug.autoScroll) {
                                            logFlickable.scrollToBottom()
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                // 右侧：状态 + 发送区
                ColumnLayout {
                    Layout.fillWidth: !root.wideLayout
                    Layout.preferredWidth: root.wideLayout ? 340 : 0
                    Layout.minimumWidth: root.wideLayout ? 300 : 0
                    Layout.alignment: Qt.AlignTop
                    spacing: 14

                    // 串口状态卡片
                    ECard {
                        Layout.fillWidth: true
                        radius: 10
                        padding: 16
                        shadowEnabled: false

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            Text {
                                text: qsTr("串口状态")
                                color: theme.textColor
                                font.pixelSize: 15
                                font.bold: true
                                Layout.fillWidth: true
                            }

                            InfoRow {
                                label: qsTr("状态")
                                value: serialDebug.isOpen ? qsTr("已连接") : qsTr("未连接")
                                valueColor: serialDebug.isOpen ? theme.focusColor : theme.textColor
                            }
                            InfoRow { label: qsTr("接收"); value: formatBytes(serialDebug.rxBytes) }
                            InfoRow { label: qsTr("发送"); value: formatBytes(serialDebug.txBytes) }
                        }
                    }

                    // 发送区卡片
                    ECard {
                        Layout.fillWidth: true
                        radius: 10
                        padding: 16
                        shadowEnabled: false

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 12

                            Text {
                                text: qsTr("发送区")
                                color: theme.textColor
                                font.pixelSize: 15
                                font.bold: true
                                Layout.fillWidth: true
                            }

                            TextArea {
                                id: sendInput
                                Layout.fillWidth: true
                                Layout.preferredHeight: 80
                                placeholderText: serialDebug.showHex ? qsTr("输入 HEX，如: 58 59 41 31") : qsTr("输入要发送的数据")
                                wrapMode: TextArea.Wrap
                                selectByMouse: true
                                color: theme.textColor
                                font.family: "Consolas"
                                font.pixelSize: 13
                                background: Rectangle {
                                    radius: 8
                                    color: Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 0.04)
                                    border.color: theme.borderColor
                                    border.width: 1
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 8

                                EButton {
                                    text: qsTr("发送")
                                    iconCharacter: "\uf1d8"
                                    size: "xs"
                                    radius: 8
                                    containerColor: theme.focusColor
                                    hoverColor: Qt.darker(theme.focusColor, 1.12)
                                    textColor: "white"
                                    iconColor: "white"
                                    enabled: serialDebug.isOpen && sendInput.text.length > 0
                                    Layout.preferredWidth: 100
                                    Layout.preferredHeight: 38
                                    onClicked: {
                                        serialDebug.send(sendInput.text)
                                        sendInput.clear()
                                    }
                                }

                                EButton {
                                    text: qsTr("清空")
                                    iconCharacter: "\uf2ed"
                                    size: "xs"
                                    radius: 8
                                    shadowEnabled: false
                                    enabled: sendInput.text.length > 0
                                    Layout.preferredWidth: 80
                                    Layout.preferredHeight: 38
                                    onClicked: sendInput.clear()
                                }

                                Item { Layout.fillWidth: true }
                            }
                        }
                    }

                    // 快捷指令卡片
                    ECard {
                        Layout.fillWidth: true
                        radius: 10
                        padding: 16
                        shadowEnabled: false

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 10

                            Text {
                                text: qsTr("快捷指令")
                                color: theme.textColor
                                font.pixelSize: 15
                                font.bold: true
                                Layout.fillWidth: true
                            }

                            GridLayout {
                                Layout.fillWidth: true
                                columns: 2
                                columnSpacing: 8
                                rowSpacing: 8

                                EButton {
                                    text: qsTr("版本帧")
                                    iconCharacter: "\uf1d8"
                                    size: "xs"
                                    radius: 8
                                    shadowEnabled: false
                                    enabled: serialDebug.isOpen
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 36
                                    onClicked: serialDebug.send("XYVH20260608225700XYVT")
                                }

                                EButton {
                                    text: qsTr("ACK")
                                    iconCharacter: "\uf1d8"
                                    size: "xs"
                                    radius: 8
                                    shadowEnabled: false
                                    enabled: serialDebug.isOpen
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 36
                                    onClicked: serialDebug.send("XYA1")
                                }

                                EButton {
                                    text: qsTr("Boot Ready")
                                    iconCharacter: "\uf1d8"
                                    size: "xs"
                                    radius: 8
                                    shadowEnabled: false
                                    enabled: serialDebug.isOpen
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 36
                                    onClicked: serialDebug.send("XYB1")
                                }

                                EButton {
                                    text: qsTr("Boot Finish")
                                    iconCharacter: "\uf1d8"
                                    size: "xs"
                                    radius: 8
                                    shadowEnabled: false
                                    enabled: serialDebug.isOpen
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 36
                                    onClicked: serialDebug.send("XYB3")
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    function formatBytes(bytes) {
        if (bytes < 1024) return bytes + " B"
        if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(1) + " KB"
        return (bytes / 1024 / 1024).toFixed(2) + " MB"
    }

    component InfoRow: RowLayout {
        property string label: ""
        property string value: ""
        property color valueColor: theme.textColor

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
            color: valueColor
            font.pixelSize: 12
            font.bold: valueColor !== theme.textColor
            horizontalAlignment: Text.AlignRight
            Layout.fillWidth: true
            elide: Text.ElideRight
        }
    }
}
