// ECheckBox.qml
import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    property string text: "Checkbox"
    property bool checked: false
    signal toggled(bool checked)

    property int fontSize: 13
    property color textColor: theme.textColor
    property color checkColor: theme.focusColor

    implicitWidth: row.implicitWidth + 16
    implicitHeight: 28
    radius: 6
    color: mouseArea.containsMouse ? Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 0.06) : "transparent"

    opacity: enabled ? 1.0 : 0.45

    Behavior on color { ColorAnimation { duration: 100 } }

    RowLayout {
        id: row
        anchors.centerIn: parent
        spacing: 8

        Rectangle {
            width: 16
            height: 16
            radius: 3
            color: root.checked ? root.checkColor : "transparent"
            border.width: 2
            border.color: root.checked ? root.checkColor : Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 0.4)

            Behavior on color { ColorAnimation { duration: 100 } }
            Behavior on border.color { ColorAnimation { duration: 100 } }

            Text {
                anchors.centerIn: parent
                text: "✓"
                font.pixelSize: 12
                font.bold: true
                color: "white"
                visible: root.checked
            }
        }

        Text {
            text: root.text
            color: root.textColor
            font.pixelSize: root.fontSize
            verticalAlignment: Text.AlignVCenter
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        enabled: root.enabled
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor

        onClicked: {
            root.checked = !root.checked
            root.toggled(root.checked)
        }
    }
}
