pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import SerialAssistant

Rectangle {
    id: root
    required property var theme
    required property var appSettings
    required property var serialBackend
    required property var transmitBackend
    property string currentSection: "serial"
    readonly property string title: sectionTitle(currentSection)
    signal closeRequested()

    radius: 14
    color: root.theme.secondaryColor
    border.width: 1
    border.color: root.theme.borderColor

    Rectangle {
        anchors.fill: parent
        anchors.margins: -5
        z: -1
        radius: 18
        color: root.theme.shadowColor
        opacity: 0.28
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 70
            Layout.leftMargin: 18
            Layout.rightMargin: 12
            spacing: 12
            MaterialIcon {
                icon: root.sectionIcon(root.currentSection)
                color: root.theme.focusColor
                font.pixelSize: 24
                Layout.preferredWidth: 28
            }
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2
                Text { text: root.title; color: root.theme.textColor; font.pixelSize: 16; font.bold: true }
                Text { text: root.sectionDescription(root.currentSection); color: root.theme.mutedTextColor; font.pixelSize: 10; elide: Text.ElideRight; Layout.fillWidth: true }
            }
            Rectangle {
                Layout.preferredWidth: 36; Layout.preferredHeight: 36
                radius: 9
                color: closeMouse.containsMouse ? root.theme.elevatedColor : "transparent"
                MaterialIcon { anchors.centerIn: parent; icon: "close"; color: root.theme.mutedTextColor; font.pixelSize: 22 }
                MouseArea { id: closeMouse; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: root.closeRequested() }
            }
        }

        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: root.theme.borderColor }

        ScrollView {
            id: settingsScroll
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: availableWidth

            ColumnLayout {
                width: settingsScroll.availableWidth
                spacing: 12
                Item { Layout.preferredHeight: 4 }

                // 串口设置
                SettingGroup {
                    theme: root.theme; Layout.fillWidth: true; Layout.leftMargin: 14; Layout.rightMargin: 14
                    visible: root.currentSection === "serial"
                    title: qsTr("通信参数")
                    description: qsTr("修改串口参数会先关闭当前连接，重新打开后生效。")
                    ValueSelect { label: qsTr("数据位"); value: root.serialBackend.dataBits; model: root.serialBackend.dataBitsOptions; onChosen: value => root.serialBackend.dataBits = Number(value) }
                    ValueSelect { label: qsTr("校验位"); value: root.serialBackend.parity; model: root.serialBackend.parityOptions; onChosen: value => root.serialBackend.parity = Number(value) }
                    ValueSelect { label: qsTr("停止位"); value: root.serialBackend.stopBits; model: root.serialBackend.stopBitsOptions; onChosen: value => root.serialBackend.stopBits = Number(value) }
                    ValueSelect { label: qsTr("流控"); value: root.serialBackend.flowControl; model: root.serialBackend.flowControlOptions; onChosen: value => root.serialBackend.flowControl = Number(value) }
                }
                SettingGroup {
                    theme: root.theme; Layout.fillWidth: true; Layout.leftMargin: 14; Layout.rightMargin: 14
                    visible: root.currentSection === "serial"
                    title: qsTr("流控状态")
                    SettingSwitch { theme: root.theme; title: "DTR"; description: qsTr("数据终端就绪信号"); checked: root.serialBackend.dtrEnabled; onCheckedChanged: root.serialBackend.dtrEnabled = checked }
                    SettingSwitch { theme: root.theme; title: "RTS"; description: qsTr("请求发送信号"); checked: root.serialBackend.rtsEnabled; onCheckedChanged: root.serialBackend.rtsEnabled = checked }
                }
                SettingGroup {
                    theme: root.theme; Layout.fillWidth: true; Layout.leftMargin: 14; Layout.rightMargin: 14
                    visible: root.currentSection === "serial"
                    title: qsTr("启动与重连")
                    SettingSwitch { theme: root.theme; title: qsTr("启动时自动打开串口"); checked: root.serialBackend.autoOpen; onCheckedChanged: root.serialBackend.autoOpen = checked }
                    SettingSwitch { theme: root.theme; title: qsTr("记忆上次端口"); checked: root.serialBackend.rememberPort; onCheckedChanged: root.serialBackend.rememberPort = checked }
                    NumberSetting { label: qsTr("断线重连间隔"); valueText: "1000"; suffix: "ms" }
                }

                // 编码规则
                SettingGroup {
                    theme: root.theme; Layout.fillWidth: true; Layout.leftMargin: 14; Layout.rightMargin: 14
                    visible: root.currentSection === "encoding"
                    title: qsTr("文本编码")
                    description: qsTr("分别指定接收数据和发送文本使用的字符编码。")
                    ValueSelect { label: qsTr("接收编码"); value: root.appSettings.receiveEncoding; model: ["UTF-8", "GBK", "ASCII", "Latin-1"]; onChosen: value => root.appSettings.receiveEncoding = String(value) }
                    ValueSelect { label: qsTr("发送编码"); value: root.appSettings.sendEncoding; model: ["UTF-8", "GBK", "ASCII", "Latin-1"]; onChosen: value => root.appSettings.sendEncoding = String(value) }
                    ValueSelect { label: qsTr("无效字节处理"); value: root.appSettings.invalidBytePolicy; model: [{ text: qsTr("替换字符"), value: 0 }, { text: qsTr("忽略"), value: 1 }, { text: qsTr("转义显示"), value: 2 }]; onChosen: value => root.appSettings.invalidBytePolicy = Number(value) }
                }
                SettingGroup {
                    theme: root.theme; Layout.fillWidth: true; Layout.leftMargin: 14; Layout.rightMargin: 14
                    visible: root.currentSection === "encoding"
                    title: qsTr("预览")
                    description: qsTr("串口助手 · Serial Assistant · 你好，设备")
                }

                // 回车样式
                SettingGroup {
                    theme: root.theme; Layout.fillWidth: true; Layout.leftMargin: 14; Layout.rightMargin: 14
                    visible: root.currentSection === "lineEnding"
                    title: qsTr("发送行尾")
                    description: qsTr("发送文本时追加到数据末尾的控制字符。")
                    SegmentedSelector { theme: root.theme; Layout.fillWidth: true; options: [qsTr("无"), "LF", "CR", "CRLF"]; currentIndex: root.transmitBackend.lineEnding; onSelected: (index, value) => { root.transmitBackend.lineEnding = index; root.appSettings.appendNewline = index === 3 } }
                }
                SettingGroup {
                    theme: root.theme; Layout.fillWidth: true; Layout.leftMargin: 14; Layout.rightMargin: 14
                    visible: root.currentSection === "lineEnding"
                    title: qsTr("输入行为")
                    SettingSwitch { theme: root.theme; title: qsTr("回车直接发送"); checked: root.appSettings.enterToSend; onCheckedChanged: root.appSettings.enterToSend = checked }
                    SettingSwitch { theme: root.theme; title: qsTr("发送后追加新行"); checked: root.appSettings.appendNewline; onCheckedChanged: { root.appSettings.appendNewline = checked; root.transmitBackend.lineEnding = checked ? 3 : 0 } }
                    SettingSwitch { theme: root.theme; title: qsTr("允许多行输入"); checked: root.appSettings.multilineInput; onCheckedChanged: root.appSettings.multilineInput = checked }
                }

                // 个性化
                SettingGroup {
                    theme: root.theme; Layout.fillWidth: true; Layout.leftMargin: 14; Layout.rightMargin: 14
                    visible: root.currentSection === "appearance"
                    title: qsTr("主题")
                    SegmentedSelector { theme: root.theme; Layout.fillWidth: true; options: [qsTr("浅色"), qsTr("深色"), qsTr("跟随系统")]; currentIndex: root.appSettings.themeMode; onSelected: (index, value) => root.appSettings.themeMode = index }
                }
                SettingGroup {
                    theme: root.theme; Layout.fillWidth: true; Layout.leftMargin: 14; Layout.rightMargin: 14
                    visible: root.currentSection === "appearance"
                    title: qsTr("强调色")
                    RowLayout {
                        Layout.fillWidth: true; spacing: 10
                        Repeater {
                            model: ["#00AFA0", "#4F7DFF", "#7C5CFF", "#E05D68", "#E89B2D"]
                            Rectangle {
                                required property var modelData
                                width: 32; height: 32; radius: 16
                                color: modelData
                                border.width: 2
                                border.color: root.appSettings.accentColor === modelData ? "white" : "transparent"
                                Rectangle {
                                    anchors.fill: parent
                                    radius: 16
                                    color: "transparent"
                                    border.width: 2
                                    border.color: root.appSettings.accentColor === modelData ? root.theme.textColor : "transparent"
                                }
                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: root.appSettings.accentColor = modelData
                                }
                            }
                        }
                        Item { Layout.fillWidth: true }
                        MaterialIcon { icon: "palette"; color: root.theme.mutedTextColor; font.pixelSize: 22 }
                    }
                }
                SettingGroup {
                    theme: root.theme; Layout.fillWidth: true; Layout.leftMargin: 14; Layout.rightMargin: 14
                    visible: root.currentSection === "appearance"
                    title: qsTr("界面与字体")
                    SegmentedSelector { theme: root.theme; Layout.fillWidth: true; options: [qsTr("紧凑"), qsTr("标准"), qsTr("宽松")]; currentIndex: root.appSettings.uiDensity; onSelected: (index, value) => root.appSettings.uiDensity = index }
                    ValueSelect { label: qsTr("终端字体"); value: root.appSettings.terminalFont; model: ["Consolas", "Cascadia Mono", "JetBrains Mono", qsTr("系统等宽")]; onChosen: value => root.appSettings.terminalFont = String(value) }
                    NumberSetting { label: qsTr("终端字号"); valueText: root.appSettings.terminalFontSize.toString(); suffix: "px"; onValueAccepted: value => root.appSettings.terminalFontSize = value }
                }

                // 关键字高亮
                SettingGroup {
                    theme: root.theme; Layout.fillWidth: true; Layout.leftMargin: 14; Layout.rightMargin: 14
                    visible: root.currentSection === "highlight"
                    title: qsTr("匹配规则")
                    SettingSwitch { theme: root.theme; title: qsTr("启用关键字高亮"); checked: root.appSettings.highlightEnabled; onCheckedChanged: root.appSettings.highlightEnabled = checked }
                    SettingSwitch { theme: root.theme; title: qsTr("区分大小写"); checked: root.appSettings.highlightCaseSensitive; onCheckedChanged: root.appSettings.highlightCaseSensitive = checked }
                    SettingSwitch { theme: root.theme; title: qsTr("使用正则表达式"); checked: root.appSettings.highlightRegex; onCheckedChanged: root.appSettings.highlightRegex = checked }
                }
                SettingGroup {
                    theme: root.theme; Layout.fillWidth: true; Layout.leftMargin: 14; Layout.rightMargin: 14
                    visible: root.currentSection === "highlight"
                    title: qsTr("高亮规则")
                    HighlightRule { keyword: "ERROR"; ruleColor: "#E05D68" }
                    HighlightRule { keyword: "WARN"; ruleColor: "#E89B2D" }
                    HighlightRule { keyword: "OK"; ruleColor: "#20A66A" }
                    EButton { theme: root.theme; text: qsTr("添加规则"); iconName: "add"; Layout.fillWidth: true; Layout.preferredHeight: 34 }
                }

                // 时间戳
                SettingGroup {
                    theme: root.theme; Layout.fillWidth: true; Layout.leftMargin: 14; Layout.rightMargin: 14
                    visible: root.currentSection === "timestamp"
                    title: qsTr("时间戳显示")
                    SettingSwitch { theme: root.theme; title: qsTr("启用时间戳"); checked: root.appSettings.timestampEnabled; onCheckedChanged: root.appSettings.timestampEnabled = checked }
                    SegmentedSelector { theme: root.theme; Layout.fillWidth: true; options: [qsTr("仅接收"), qsTr("收发全部")]; currentIndex: root.appSettings.timestampScope; onSelected: (index, value) => root.appSettings.timestampScope = index }
                    ValueSelect { label: qsTr("时间格式"); value: root.appSettings.timestampFormat; model: ["HH:mm:ss.SSS", "HH:mm:ss", "yyyy-MM-dd HH:mm:ss"]; onChosen: value => root.appSettings.timestampFormat = String(value) }
                    SettingSwitch { theme: root.theme; title: qsTr("显示毫秒"); checked: root.appSettings.timestampFormat.indexOf("SSS") >= 0; onCheckedChanged: root.appSettings.timestampFormat = checked ? "HH:mm:ss.SSS" : "HH:mm:ss" }
                }
                SettingGroup {
                    theme: root.theme; Layout.fillWidth: true; Layout.leftMargin: 14; Layout.rightMargin: 14
                    visible: root.currentSection === "timestamp"
                    title: qsTr("分包规则")
                    description: qsTr("超过指定空闲时间后，将下一批数据视为新的接收记录。")
                    NumberSetting { label: qsTr("分包超时"); valueText: "20"; suffix: "ms" }
                }

                // 恢复默认
                SettingGroup {
                    theme: root.theme; Layout.fillWidth: true; Layout.leftMargin: 14; Layout.rightMargin: 14
                    visible: root.currentSection === "restore"
                    title: qsTr("恢复默认设置")
                    description: qsTr("将当前已接入的串口参数、显示规则和发送选项恢复到初始状态。")
                    RowLayout {
                        Layout.fillWidth: true; spacing: 10
                        MaterialIcon { icon: "warning"; color: root.theme.dangerColor; font.pixelSize: 24 }
                        Text { Layout.fillWidth: true; text: qsTr("此操作不会删除接收记录或日志文件。"); color: root.theme.mutedTextColor; font.pixelSize: 11; wrapMode: Text.Wrap }
                    }
                    EButton { theme: root.theme; text: qsTr("恢复全部默认设置"); iconName: "restart_alt"; containerColor: root.theme.dangerColor; textColor: root.theme.dangerColor; Layout.fillWidth: true; Layout.preferredHeight: 38; onClicked: resetConfirm.open() }
                }

                Item { Layout.preferredHeight: 10 }
            }
        }
    }

    Popup {
        id: resetConfirm
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 360
        height: 210
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        background: Rectangle { radius: 12; color: root.theme.secondaryColor; border.width: 1; border.color: root.theme.borderColor }
        contentItem: ColumnLayout {
            spacing: 12
            MaterialIcon { icon: "restart_alt"; color: root.theme.dangerColor; font.pixelSize: 32; Layout.alignment: Qt.AlignHCenter }
            Text { text: qsTr("确认恢复默认设置？"); color: root.theme.textColor; font.pixelSize: 15; font.bold: true; Layout.alignment: Qt.AlignHCenter }
            Text { text: qsTr("已接入的串口、显示和发送设置会立即恢复默认值。"); color: root.theme.mutedTextColor; font.pixelSize: 11; Layout.alignment: Qt.AlignHCenter }
            RowLayout {
                Layout.fillWidth: true; spacing: 10
                EButton { theme: root.theme; text: qsTr("取消"); Layout.fillWidth: true; onClicked: resetConfirm.close() }
                EButton { theme: root.theme; text: qsTr("确认恢复"); primary: true; containerColor: root.theme.dangerColor; Layout.fillWidth: true; onClicked: { if (root.serialBackend.isOpen) root.serialBackend.closePort(); root.appSettings.restoreDefaults(); resetConfirm.close() } }
            }
        }
    }

    function sectionTitle(section) {
        const titles = { serial: qsTr("串口设置"), encoding: qsTr("编码规则"), lineEnding: qsTr("回车样式"), appearance: qsTr("个性化"), highlight: qsTr("关键字高亮"), timestamp: qsTr("时间戳设置"), restore: qsTr("恢复默认设置") }
        return titles[section] || qsTr("设置")
    }
    function sectionDescription(section) {
        const descriptions = { serial: qsTr("通信参数、流控与自动连接"), encoding: qsTr("接收和发送文本的字符编码"), lineEnding: qsTr("行尾字符与输入行为"), appearance: qsTr("主题、颜色、密度与字体"), highlight: qsTr("关键字匹配与颜色规则"), timestamp: qsTr("时间格式和数据分包显示"), restore: qsTr("查看并恢复初始配置") }
        return descriptions[section] || ""
    }
    function sectionIcon(section) {
        const icons = { serial: "settings_input_component", encoding: "translate", lineEnding: "keyboard_return", appearance: "palette", highlight: "format_color_fill", timestamp: "schedule", restore: "restart_alt" }
        return icons[section] || "settings"
    }

    component ValueSelect: RowLayout {
        id: valueSelect
        property string label: ""
        property var value: ""
        property var model: []
        signal chosen(var value)
        Layout.fillWidth: true
        spacing: 12
        Text { Layout.fillWidth: true; text: valueSelect.label; color: root.theme.textColor; font.pixelSize: 12 }
        EDropdown { theme: root.theme; Layout.preferredWidth: 190; Layout.preferredHeight: 34; model: valueSelect.model; selectedValue: valueSelect.value; onValueSelected: value => valueSelect.chosen(String(value)) }
    }
    component NumberSetting: RowLayout {
        id: numberSetting
        property string label: ""
        property string valueText: ""
        property string suffix: ""
        signal valueAccepted(int value)
        Layout.fillWidth: true
        spacing: 8
        Text { Layout.fillWidth: true; text: numberSetting.label; color: root.theme.textColor; font.pixelSize: 12 }
        EInput {
            theme: root.theme
            text: numberSetting.valueText
            Layout.preferredWidth: 82
            Layout.preferredHeight: 32
            validator: IntValidator { bottom: 0; top: 99999 }
            onEditingFinished: numberSetting.valueAccepted(Number(text))
        }
        Text { text: numberSetting.suffix; color: root.theme.mutedTextColor; font.pixelSize: 11; Layout.preferredWidth: 24 }
    }
    component HighlightRule: RowLayout {
        id: rule
        property string keyword: ""
        property color ruleColor: "#00AFA0"
        Layout.fillWidth: true
        spacing: 8
        Rectangle { Layout.preferredWidth: 20; Layout.preferredHeight: 20; radius: 6; color: rule.ruleColor }
        EInput { theme: root.theme; text: rule.keyword; Layout.fillWidth: true; Layout.preferredHeight: 32 }
        Rectangle {
            Layout.preferredWidth: 32; Layout.preferredHeight: 32; radius: 8; color: deleteMouse.containsMouse ? root.theme.elevatedColor : "transparent"
            MaterialIcon { anchors.centerIn: parent; icon: "delete"; color: root.theme.mutedTextColor; font.pixelSize: 19 }
            MouseArea { id: deleteMouse; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor }
        }
    }
}
