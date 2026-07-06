// FilterHighlightCard — 侧栏中的过滤/高亮卡片。
// 自包含：关键字过滤、高亮、HEX 显示、自动滚动等设置项。
import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import SerialAssistant

Rectangle {
    id: root

    required property var theme

    // 与 Main.qml 应用窗口双向绑定的状态
    property bool keywordFilter: false
    property bool keywordHighlight: false
    property string keywordText: ""
    property bool hexDisplay: false
    property bool autoScroll: true

    color: root.theme.isDark ? root.theme.elevatedColor : "#FBFCFE"
    radius: 10
    border.width: 1
    border.color: root.theme.borderColor

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 7

        Text {
            text: qsTr("过滤 / 高亮")
            color: root.theme.textColor
            font.pixelSize: 13
            font.bold: true
            font.letterSpacing: 0.5
        }

        EOptRow {
            Layout.fillWidth: true
            label: qsTr("仅显示关键字")
            checked: root.keywordFilter
            theme: root.theme
            onCheckedChanged: root.keywordFilter = checked
        }
        EOptRow {
            Layout.fillWidth: true
            label: qsTr("关键字高亮")
            checked: root.keywordHighlight
            theme: root.theme
            onCheckedChanged: root.keywordHighlight = checked
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4
            Text {
                text: qsTr("关键字")
                color: root.theme.mutedTextColor
                font.pixelSize: 11
            }
            EInput {
                theme: root.theme
                Layout.fillWidth: true
                implicitHeight: 28
                text: root.keywordText
                placeholderText: qsTr("如 AT+,ERR")
                onTextChanged: root.keywordText = text
            }
        }

        Text {
            text: qsTr("显示选项")
            color: root.theme.mutedTextColor
            font.pixelSize: 11
            font.bold: true
            font.letterSpacing: 0.5
            Layout.topMargin: 6
        }

        EOptRow {
            Layout.fillWidth: true
            label: qsTr("HEX 显示")
            checked: root.hexDisplay
            theme: root.theme
            onCheckedChanged: root.hexDisplay = checked
        }
        EOptRow {
            Layout.fillWidth: true
            label: qsTr("时间戳")
            checked: true
            enabled: false
            theme: root.theme
        }
        EOptRow {
            Layout.fillWidth: true
            label: qsTr("自动滚动")
            checked: root.autoScroll
            theme: root.theme
            onCheckedChanged: root.autoScroll = checked
        }
    }
}
