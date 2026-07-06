import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import SerialAssistant

EPaneCard {
    id: root

    property bool hexSend: false
    property bool hexDisplay: false
    property bool timestampEnabled: true
    property bool loopSend: false
    property int loopIntervalMs: 200
    property bool enterToSend: false
    property bool appendNewline: false
    property bool multilineInput: true
    property string terminalFont: "Consolas"
    property int terminalFontSize: 12
    property int uiDensity: 1
    property alias sendText: sendTextArea.text
    property int byteCount: 0
    property bool isPortOpen: false

    signal cleared()
    signal sendRequested()
    signal sendSettingChanged(string name, var value)
    signal timestampSettingChanged(bool enabled)

    radius: 12

    Item {
        anchors.fill: parent
        anchors.margins: 12

        Rectangle {
            id: inputPanel
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: optionPanel.left
            anchors.rightMargin: 14
            radius: 8
            color: root.theme.isDark ? "#1D2128" : "#FBFCFE"
            border.width: 1
            border.color: root.theme.borderColor

            ScrollView {
                anchors.fill: parent
                clip: true
                TextArea {
                    id: sendTextArea
                    placeholderText: qsTr("数据发送区（支持多行文本输入）")
                    placeholderTextColor: root.theme.mutedTextColor
                    selectByMouse: true
                    wrapMode: TextArea.Wrap
                    color: root.theme.textColor
                    selectionColor: root.theme.focusColor
                    selectedTextColor: "white"
                    font.family: root.terminalFont
                    font.pixelSize: root.terminalFontSize
                    padding: root.uiDensity === 0 ? 8 : (root.uiDensity === 2 ? 12 : 10)
                    background: null

                    Keys.onPressed: event => {
                        const isEnter = event.key === Qt.Key_Return || event.key === Qt.Key_Enter
                        const wantsSend = root.enterToSend || (event.modifiers & Qt.ControlModifier)
                        if (isEnter && wantsSend) {
                            root.sendRequested()
                            event.accepted = true
                        } else if (isEnter && !root.multilineInput) {
                            event.accepted = true
                        }
                    }
                }
            }
        }

        GridLayout {
            id: optionPanel
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: actionPanel.left
            anchors.rightMargin: 14
            width: 330
            columns: 2
            rows: 4
            rowSpacing: 5
            columnSpacing: 12

            EOptRow {
                Layout.preferredWidth: 125
                Layout.minimumWidth: 125
                theme: root.theme; label: qsTr("时间戳"); checked: root.timestampEnabled
                onCheckedChanged: {
                    root.timestampEnabled = checked
                    root.timestampSettingChanged(checked)
                }
            }
            RowLayout {
                Layout.preferredWidth: 190
                Layout.minimumWidth: 190
                spacing: 5
                EInput { theme: root.theme; text: "20"; Layout.preferredWidth: 60; Layout.preferredHeight: 28 }
                Text { text: qsTr("ms超时"); color: root.theme.textColor; font.pixelSize: 11 }
                Item { Layout.fillWidth: true }
            }
            EOptRow {
                Layout.preferredWidth: 125
                Layout.minimumWidth: 125
                theme: root.theme; label: qsTr("循环发送"); checked: root.loopSend
                onCheckedChanged: { root.loopSend = checked; root.sendSettingChanged("loopSend", checked) }
            }
            RowLayout {
                Layout.preferredWidth: 190
                Layout.minimumWidth: 190
                spacing: 5
                EInput {
                    theme: root.theme
                    text: root.loopIntervalMs.toString()
                    Layout.preferredWidth: 60
                    Layout.preferredHeight: 28
                    validator: IntValidator { bottom: 10; top: 60000 }
                    onEditingFinished: root.sendSettingChanged("loopIntervalMs", Number(text))
                }
                Text { text: qsTr("ms/次"); color: root.theme.textColor; font.pixelSize: 11 }
                Item { Layout.fillWidth: true }
            }
            EOptRow {
                Layout.preferredWidth: 125
                Layout.minimumWidth: 125
                theme: root.theme; label: qsTr("HEX显示"); checked: root.hexDisplay
                onCheckedChanged: root.hexDisplay = checked
            }
            EOptRow {
                Layout.preferredWidth: 190
                Layout.minimumWidth: 190
                theme: root.theme; label: qsTr("HEX发送"); checked: root.hexSend
                onCheckedChanged: { root.hexSend = checked; root.sendSettingChanged("hexSend", checked) }
            }
            EOptRow {
                Layout.preferredWidth: 125
                Layout.minimumWidth: 125
                theme: root.theme; label: qsTr("回车发送"); checked: root.enterToSend
                onCheckedChanged: { root.enterToSend = checked; root.sendSettingChanged("enterToSend", checked) }
            }
            EOptRow {
                Layout.preferredWidth: 190
                Layout.minimumWidth: 190
                theme: root.theme; label: qsTr("追加新行"); checked: root.appendNewline
                onCheckedChanged: { root.appendNewline = checked; root.sendSettingChanged("appendNewline", checked) }
            }
        }

        ColumnLayout {
            id: actionPanel
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            width: 150
            spacing: 10
            EButton {
                theme: root.theme
                text: qsTr("发送")
                iconName: "send"
                primary: true
                Layout.fillWidth: true
                Layout.preferredHeight: 54
                enabled: root.isPortOpen && root.sendText.length > 0
                onClicked: root.sendRequested()
            }
            EButton {
                theme: root.theme
                text: qsTr("清空")
                iconName: "cleaning_services"
                Layout.fillWidth: true
                Layout.preferredHeight: 54
                onClicked: root.cleared()
            }
            Item { Layout.fillHeight: true }
        }
    }
}
