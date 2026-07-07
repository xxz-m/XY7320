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

    onKeywordFilterChanged: updateModelFilter()
    onKeywordTextChanged: updateModelFilter()
    onHighlightCaseSensitiveChanged: updateModelFilter()
    onHighlightRegexChanged: updateModelFilter()
    onRecordModelChanged: updateModelFilter()

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

            Flickable {
                id: recordFlickable
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.topMargin: 10
                anchors.bottomMargin: 10
                anchors.rightMargin: 18
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                contentWidth: width
                contentHeight: Math.max(height, receiveTextEdit.paintedHeight)

                TextEdit {
                    id: receiveTextEdit
                    width: recordFlickable.width
                    readOnly: true
                    selectByMouse: true
                    selectByKeyboard: true
                    persistentSelection: true
                    wrapMode: TextEdit.WrapAnywhere
                    textFormat: TextEdit.RichText
                    text: root.buildDisplayRichText(root.recordModel ? root.recordModel.displayRichText : "")
                    color: root.theme.textColor
                    selectedTextColor: root.theme.isDark ? "#0F172A" : "#FFFFFF"
                    selectionColor: root.theme.focusColor
                    font.family: root.terminalFont
                    font.pixelSize: root.terminalFontSize

                    onTextChanged: if (root.autoScroll) Qt.callLater(() => recordFlickable.contentY = Math.max(0, recordFlickable.contentHeight - recordFlickable.height))
                }

                Text {
                    anchors.centerIn: parent
                    visible: receiveTextEdit.length === 0
                    text: qsTr("等待串口数据…")
                    color: root.theme.mutedTextColor
                    font.pixelSize: 12
                }
            }

            Rectangle {
                id: receiveScrollTrack
                anchors.top: parent.top
                anchors.topMargin: 12
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 12
                anchors.right: parent.right
                anchors.rightMargin: 7
                width: 9
                radius: width / 2
                color: Qt.rgba(root.theme.borderColor.r, root.theme.borderColor.g, root.theme.borderColor.b, 0.34)
                visible: receiveTextEdit.paintedHeight > recordFlickable.height

                MouseArea {
                    anchors.fill: parent
                    onClicked: mouse => {
                        const targetY = Math.max(0, Math.min(mouse.y - receiveScrollThumb.height / 2,
                                                            receiveScrollTrack.height - receiveScrollThumb.height))
                        recordFlickable.contentY = targetY * Math.max(recordFlickable.contentHeight - recordFlickable.height, 0)
                                                   / Math.max(receiveScrollTrack.height - receiveScrollThumb.height, 1)
                    }
                }

                Rectangle {
                    id: receiveScrollThumb
                    width: parent.width
                    radius: width / 2
                    color: thumbDragArea.pressed
                           ? root.theme.focusColor
                           : Qt.rgba(root.theme.focusColor.r, root.theme.focusColor.g, root.theme.focusColor.b, thumbDragArea.containsMouse ? 0.88 : 0.72)
                    height: Math.max(34, parent.height * recordFlickable.height / Math.max(recordFlickable.contentHeight, 1))
                    y: (parent.height - height) * recordFlickable.contentY / Math.max(recordFlickable.contentHeight - recordFlickable.height, 1)

                    MouseArea {
                        id: thumbDragArea
                        anchors.fill: parent
                        hoverEnabled: true
                        drag.target: parent
                        drag.axis: Drag.YAxis
                        drag.minimumY: 0
                        drag.maximumY: receiveScrollTrack.height - receiveScrollThumb.height
                        onPositionChanged: if (pressed) {
                            recordFlickable.contentY = receiveScrollThumb.y * Math.max(recordFlickable.contentHeight - recordFlickable.height, 0)
                                                       / Math.max(receiveScrollTrack.height - receiveScrollThumb.height, 1)
                        }
                    }
                }

            }
        }
    }

    function updateModelFilter() {
        if (!root.recordModel || !root.recordModel.setDisplayFilter)
            return
        root.recordModel.setDisplayFilter(root.keywordFilter,
                                          root.keywordText,
                                          root.highlightCaseSensitive,
                                          root.highlightRegex)
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

    function buildDisplayRichText(richText) {
        const numberColor = root.theme.isDark ? "#FFD166" : "#B45309"
        const timeColor = "#8B8DA8"
        let html = "<style>.num{color:" + numberColor + ";font-weight:600}.time{color:" + timeColor + "}</style>" + richText

        const kws = keywordList()
        if (!root.keywordHighlight || kws.length === 0)
            return html

        const keywordColor = root.theme.isDark ? "#7DD3FC" : "#0E7490"
        const flags = root.highlightCaseSensitive ? "g" : "gi"
        try {
            const pattern = root.highlightRegex ? kws.join("|") : kws.map(escapeRegExp).join("|")
            if (pattern.length === 0) return html
            return html.replace(new RegExp(pattern, flags), "<span style=\"color:" + keywordColor + "; font-weight:700;\">$&</span>")
        } catch (error) {
            return html
        }
    }

    function formatRecordText(text) {
        let escaped = escapeHtml(text)
        const numberColor = root.theme.isDark ? "#FFD166" : "#B45309"
        escaped = escaped.replace(/([0-9]+)/g, "<span style=\"color:" + numberColor + "; font-weight:600;\">$1</span>")
        return escaped
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
