pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import SerialAssistant

EPaneCard {
    id: root

    property var recordModel
    property bool timestampEnabled: true
    property int timestampScope: 0
    property bool autoScroll: true
    property bool receivePaused: false
    property bool keywordFilter: false
    property bool keywordHighlight: false
    property bool highlightCaseSensitive: false
    property bool highlightRegex: false
    property string keywordText: ""
    property bool hexDisplay: false
    property string terminalFont: "Consolas"
    property int terminalFontSize: 12
    property int uiDensity: 1

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
                anchors.leftMargin: 10
                anchors.topMargin: 10
                anchors.bottomMargin: 10
                anchors.rightMargin: 18
                clip: true
                model: root.recordModel
                spacing: root.uiDensity === 0 ? 1 : (root.uiDensity === 2 ? 5 : 3)

                delegate: Item {
                    id: recordDelegate
                    required property string recordTime
                    required property string direction
                    required property string content
                    required property string hexText
                    required property string directionColor
                    width: recordView.width
                    height: visible ? Math.max(root.uiDensity === 0 ? 20 : (root.uiDensity === 2 ? 28 : 24), row.implicitHeight) : 0
                    visible: !root.keywordFilter || root.keywordText.length === 0 || root.matchesKeyword(content)

                    RowLayout {
                        id: row
                        anchors.left: parent.left
                        anchors.right: parent.right
                        spacing: 8
                        Text {
                            visible: root.timestampEnabled && (root.timestampScope === 1 || recordDelegate.direction !== "tx")
                            text: recordDelegate.recordTime
                            color: "#8B8DA8"
                            font.family: root.terminalFont
                            font.pixelSize: root.terminalFontSize
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
                            textFormat: Text.RichText
                            text: root.formatRecordText(root.hexDisplay ? recordDelegate.hexText : recordDelegate.content)
                            color: root.theme.textColor
                            font.family: root.terminalFont
                            font.pixelSize: root.terminalFontSize
                            wrapMode: Text.WrapAnywhere
                        }
                    }
                }

                onCountChanged: if (root.autoScroll) Qt.callLater(() => recordView.positionViewAtEnd())

                ScrollBar.vertical: ScrollBar {
                    id: receiveScrollBar
                    policy: ScrollBar.AlwaysOn
                    width: 8
                    anchors.right: parent.right
                    anchors.rightMargin: -12
                    background: Rectangle {
                        radius: 4
                        color: Qt.rgba(root.theme.borderColor.r, root.theme.borderColor.g, root.theme.borderColor.b, 0.36)
                    }
                    contentItem: Rectangle {
                        implicitWidth: 8
                        radius: 4
                        color: receiveScrollBar.pressed
                               ? root.theme.focusColor
                               : Qt.rgba(root.theme.focusColor.r, root.theme.focusColor.g, root.theme.focusColor.b, receiveScrollBar.hovered ? 0.82 : 0.58)
                    }
                }

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

    function escapeHtml(text) {
        return String(text).replace(/&/g, "&amp;")
                           .replace(/</g, "&lt;")
                           .replace(/>/g, "&gt;")
                           .replace(/\"/g, "&quot;")
                           .replace(/'/g, "&#39;")
    }

    function keywordList() {
        return root.keywordText.split(",").map(s => s.trim()).filter(s => s.length > 0)
    }

    function escapeRegExp(text) {
        return String(text).replace(/[.*+?^${}()|[\]\\]/g, "\\$&")
    }

    function formatRecordText(text) {
        let escaped = escapeHtml(text)
        const numberColor = root.theme.isDark ? "#FFD166" : "#B45309"
        escaped = escaped.replace(/([0-9]+)/g, "<span style=\"color:" + numberColor + "; font-weight:600;\">$1</span>")

        const kws = keywordList()
        if (!root.keywordHighlight || kws.length === 0)
            return escaped

        const keywordColor = root.theme.isDark ? "#7DD3FC" : "#0E7490"
        const flags = root.highlightCaseSensitive ? "g" : "gi"
        try {
            const pattern = root.highlightRegex ? kws.join("|") : kws.map(escapeRegExp).join("|")
            if (pattern.length === 0) return escaped
            return escaped.replace(new RegExp(pattern, flags), "<span style=\"color:" + keywordColor + "; font-weight:700;\">$&</span>")
        } catch (error) {
            return escaped
        }
    }

    function matchesKeyword(text) {
        const kws = keywordList()
        if (kws.length === 0) return true
        const source = String(text)
        if (root.highlightRegex) {
            const flags = root.highlightCaseSensitive ? "" : "i"
            try {
                return kws.some(k => new RegExp(k, flags).test(source))
            } catch (error) {
                return true
            }
        }
        const haystack = root.highlightCaseSensitive ? source : source.toLowerCase()
        return kws.some(k => haystack.indexOf(root.highlightCaseSensitive ? k : k.toLowerCase()) >= 0)
    }
}
