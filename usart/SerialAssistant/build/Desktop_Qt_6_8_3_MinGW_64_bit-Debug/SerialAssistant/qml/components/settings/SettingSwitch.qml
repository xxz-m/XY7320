import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

Rectangle {
    id: root
    required property var theme
    property string title: ""
    property string description: ""
    property bool checked: false
    property bool available: true

    implicitHeight: description.length > 0 ? 52 : 38
    color: "transparent"
    opacity: available ? 1 : 0.5

    RowLayout {
        anchors.fill: parent
        spacing: 12
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2
            Text { text: root.title; color: root.theme.textColor; font.pixelSize: 12 }
            Text {
                Layout.fillWidth: true
                text: root.description
                color: root.theme.mutedTextColor
                font.pixelSize: 10
                visible: text.length > 0
                elide: Text.ElideRight
            }
        }
        Switch {
            id: control
            checked: root.checked
            enabled: root.available
            onToggled: root.checked = checked
            indicator: Rectangle {
                implicitWidth: 40
                implicitHeight: 22
                radius: 11
                color: control.checked ? root.theme.focusColor : root.theme.borderColor
                Rectangle {
                    width: 16; height: 16; radius: 8
                    x: control.checked ? parent.width - width - 3 : 3
                    anchors.verticalCenter: parent.verticalCenter
                    color: "white"
                    Behavior on x { NumberAnimation { duration: 120 } }
                }
            }
            contentItem: Item {}
        }
    }
}
