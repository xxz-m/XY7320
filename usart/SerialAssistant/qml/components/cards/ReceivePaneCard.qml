pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import SerialAssistant

EPaneCard {
    id: root

    property var recordModel
    property bool timestampEnabled: true
    property bool autoScroll: true
    property bool receivePaused: false
    property bool keywordFilter: false
    property string keywordText: ""
    property bool hexDisplay: false

    signal pauseToggled()
    signal exportRequested()
    signal cleared()

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 46
            Layout.leftMargin: 14
            Layout.rightMargin: 14
            spacing: 8

            Text {
                text: qsTr("数据接收区")
                color: root.theme.textColor
                font.pixelSize: 13
                font.bold: true
            }
            Rectangle {
                Layout.preferredHeight: 20
                Layout.preferredWidth: receiveBadge.implicitWidth + 18
                radius: 10
                color: root.theme.elevatedColor
                Text {
                    id: receiveBadge
                    anchors.centerIn: parent
                    text: root.hexDisplay && root.timestampEnabled ? qsTr("HEX + 时间戳")
                          : (root.hexDisplay ? qsTr("HEX") : (root.timestampEnabled ? qsTr("时间戳") : qsTr("文本")))
                    color: root.theme.mutedTextColor
                    font.pixelSize: 10
                }
            }
            Item { Layout.fillWidth: true }
            EButton {
                theme: root.theme
                text: root.receivePaused ? qsTr("继续") : qsTr("暂停")
                iconName: root.receivePaused ? "play_arrow" : "pause"
                Layout.preferredWidth: 68
                Layout.preferredHeight: 30
                onClicked: root.pauseToggled()
            }
            EButton {
                theme: root.theme
                text: qsTr("导出")
                iconName: "upload"
                Layout.preferredWidth: 68
                Layout.preferredHeight: 30
                onClicked: root.exportRequested()
            }
            EButton {
                theme: root.theme
                text: qsTr("清空")
                iconName: "delete"
                Layout.preferredWidth: 68
                Layout.preferredHeight: 30
                onClicked: root.cleared()
            }
        }

        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: root.theme.borderColor }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 10
            radius: 8
            color: root.theme.isDark ? "#1D2128" : "#FBFCFE"
            border.width: 1
            border.color: root.theme.borderColor

            ListView {
                id: recordView
                anchors.fill: parent
                anchors.margins: 10
                clip: true
                model: root.recordModel
                spacing: 3

                delegate: Item {
                    id: recordDelegate
                    required property string recordTime
                    required property string direction
                    required property string content
                    required property string hexText
                    required property string directionColor
                    width: recordView.width
                    height: visible ? Math.max(24, row.implicitHeight) : 0
                    visible: !root.keywordFilter || root.keywordText.length === 0 || root.matchesKeyword(content)

                    RowLayout {
                        id: row
                        anchors.left: parent.left
                        anchors.right: parent.right
                        spacing: 8
                        Text {
                            visible: root.timestampEnabled
                            text: recordDelegate.recordTime
                            color: "#8B8DA8"
                            font.family: "Consolas"
                            font.pixelSize: 12
                            Layout.preferredWidth: 92
                        }
                        MaterialIcon {
                            icon: recordDelegate.direction === "rx" ? "arrow_back" : (recordDelegate.direction === "tx" ? "arrow_forward" : "info")
                            color: recordDelegate.directionColor
                            font.pixelSize: 17
                            Layout.preferredWidth: 20
                        }
                        Text {
                            Layout.fillWidth: true
                            text: root.hexDisplay ? recordDelegate.hexText : recordDelegate.content
                            color: root.theme.textColor
                            font.family: "Consolas"
                            font.pixelSize: 12
                            wrapMode: Text.WrapAnywhere
                        }
                    }
                }

                onCountChanged: if (root.autoScroll) Qt.callLater(() => recordView.positionViewAtEnd())

                Text {
                    anchors.centerIn: parent
                    visible: recordView.count === 0
                    text: qsTr("等待串口数据…")
                    color: root.theme.mutedTextColor
                    font.pixelSize: 12
                }
            }
        }
    }

    function matchesKeyword(text) {
        const kws = root.keywordText.split(",").map(s => s.trim()).filter(s => s.length > 0)
        if (kws.length === 0) return true
        return kws.some(k => text.indexOf(k) >= 0)
    }
}
