import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic
import QtQuick.Layouts
import XY7320Host

Item {
    id: root

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 14

        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            spacing: 12

            Text {
                text: qsTr("升级日志")
                color: theme.textColor
                font.pixelSize: 22
                font.bold: true
                Layout.fillWidth: true
                elide: Text.ElideRight
            }

            EButton {
                text: qsTr("清空")
                iconCharacter: "\uf2ed"
                size: "xs"
                radius: 8
                shadowEnabled: false
                enabled: !firmwareUploader.busy
                Layout.preferredWidth: 96
                Layout.preferredHeight: 38
                onClicked: firmwareUploader.clearLog()
            }
        }

        ECard {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 10
            padding: 0
            shadowEnabled: false

            Flickable {
                id: flickable
                anchors.fill: parent
                anchors.margins: 8
                contentWidth: width
                contentHeight: logArea.implicitHeight
                clip: true
                boundsBehavior: Flickable.StopAtBounds

                // 自动滚动到底部
                function scrollToBottom() {
                    contentY = Math.max(0, contentHeight - height)
                }

                // 监听内容变化，自动滚动
                onContentHeightChanged: {
                    // 只有当用户在底部时才自动滚动
                    if (contentY >= contentHeight - height - 20) {
                        scrollToBottom()
                    }
                }

                ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AsNeeded
                    width: 8
                }

                TextArea {
                    id: logArea
                    width: flickable.width
                    height: implicitHeight
                    text: firmwareUploader.logText
                    readOnly: true
                    wrapMode: TextArea.Wrap
                    selectByMouse: true
                    color: theme.textColor
                    placeholderText: qsTr("升级日志会显示在这里。")
                    font.family: "Consolas"
                    font.pixelSize: 13
                    textMargin: 0
                    background: Rectangle {
                        color: "transparent"
                    }

                    // 监听文本变化，触发自动滚动
                    onTextChanged: {
                        flickable.scrollToBottom()
                    }
                }
            }
        }
    }
}
