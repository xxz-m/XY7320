// SendSettingsCard — 发送设置子卡片（嵌在 SendPaneCard 左侧）
// 含 HEX 发送、循环发送、回车发送、追加换行等设置项 + 循环间隔 Slider。
import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import SerialAssistant

Rectangle {
    id: root

    required property var theme

    // 与应用窗口双向绑定的状态
    property bool hexSend: false
    property bool loopSend: false
    property int loopIntervalMs: 1000
    property bool enterToSend: false
    property bool appendNewline: false
    property bool clearBeforeSend: false

    color: root.theme.primaryColor

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 6

        Text {
            text: qsTr("发送设置")
            color: root.theme.mutedTextColor
            font.pixelSize: 11
            font.bold: true
            font.letterSpacing: 0.5
        }

        EOptRow {
            Layout.fillWidth: true
            label: qsTr("HEX 发送")
            checked: root.hexSend
            theme: root.theme
            onCheckedChanged: root.hexSend = checked
        }
        EOptRow {
            Layout.fillWidth: true
            label: qsTr("循环发送")
            checked: root.loopSend
            theme: root.theme
            onCheckedChanged: root.loopSend = checked
        }

        // 循环间隔 Slider + 智能单位显示
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4
            Text {
                text: qsTr("循环间隔")
                color: root.theme.mutedTextColor
                font.pixelSize: 11
            }
            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                Slider {
                    id: intervalSlider
                    Layout.fillWidth: true
                    Layout.preferredHeight: 24
                    from: 10
                    to: 60000
                    stepSize: 10
                    value: root.loopIntervalMs
                    onValueChanged: root.loopIntervalMs = value

                    background: Rectangle {
                        x: intervalSlider.leftPadding
                        y: intervalSlider.topPadding
                           + intervalSlider.availableHeight / 2 - height / 2
                        implicitWidth: 200
                        implicitHeight: 4
                        width: intervalSlider.availableWidth
                        height: implicitHeight
                        radius: 2
                        color: root.theme.elevatedColor
                        border.color: root.theme.borderColor
                        border.width: 1
                        Rectangle {
                            width: intervalSlider.visualPosition * parent.width
                            height: parent.height
                            radius: 2
                            color: root.theme.focusColor
                        }
                    }
                    handle: Rectangle {
                        x: intervalSlider.leftPadding
                           + intervalSlider.visualPosition
                           * (intervalSlider.availableWidth - width)
                        y: intervalSlider.topPadding
                           + intervalSlider.availableHeight / 2 - height / 2
                        implicitWidth: 16
                        implicitHeight: 16
                        radius: 8
                        color: intervalSlider.pressed ? root.theme.focusColor : "white"
                        border.color: root.theme.focusColor
                        border.width: 2
                        Behavior on color { ColorAnimation { duration: 120 } }
                    }
                }
                Text {
                    Layout.preferredWidth: 56
                    horizontalAlignment: Text.AlignRight
                    text: {
                        const v = root.loopIntervalMs
                        if (v >= 1000)
                            return (v / 1000).toFixed(v % 1000 === 0 ? 0 : 1) + "s"
                        return v + "ms"
                    }
                    color: root.theme.textColor
                    font.pixelSize: 12
                    font.family: "Consolas"
                }
            }
        }

        EOptRow {
            Layout.fillWidth: true
            label: qsTr("回车发送")
            checked: root.enterToSend
            theme: root.theme
            onCheckedChanged: root.enterToSend = checked
        }
        EOptRow {
            Layout.fillWidth: true
            label: qsTr("追加换行")
            checked: root.appendNewline
            theme: root.theme
            onCheckedChanged: root.appendNewline = checked
        }
        EOptRow {
            Layout.fillWidth: true
            label: qsTr("发送前清空")
            checked: false
            enabled: false
            theme: root.theme
        }

        Item { Layout.fillHeight: true }

        Text {
            text: qsTr("Ctrl+Enter 发送")
            color: root.theme.mutedTextColor
            font.pixelSize: 10
        }
    }
}
