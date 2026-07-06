// Adapted from EvolveUI under the MIT license.
import QtQuick

Rectangle {
    id: root

    required property var theme
    property string text: "按钮"
    property string iconName: ""
    property color containerColor: root.theme.elevatedColor
    property color hoverColor: Qt.darker(containerColor, 1.06)
    property color textColor: root.theme.textColor
    property bool primary: false
    signal clicked()

    // 胶囊型：圆角 = 高度 / 2，随高度自动适应
    property real pillRadius: Math.max(0, height / 2)
    radius: pillRadius

    implicitHeight: 40
    implicitWidth: Math.max(96, label.implicitWidth + 36)
    color: primary ? theme.focusColor : (mouseArea.containsMouse ? hoverColor : containerColor)
    border.width: primary ? 0 : 1
    border.color: theme.borderColor
    opacity: enabled ? 1 : 0.45
    scale: mouseArea.pressed ? 0.97 : 1

    Behavior on color { ColorAnimation { duration: 120 } }
    Behavior on scale { NumberAnimation { duration: 90 } }

    Row {
        anchors.centerIn: parent
        spacing: 7
        MaterialIcon {
            anchors.verticalCenter: parent.verticalCenter
            visible: root.iconName.length > 0
            icon: root.iconName
            color: root.primary ? "white" : root.textColor
            font.pixelSize: 18
            width: visible ? 18 : 0
            height: 18
        }
        Text {
            id: label
            anchors.verticalCenter: parent.verticalCenter
            text: root.text
            color: root.primary ? "white" : root.textColor
            font.pixelSize: 13
            font.bold: root.primary
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        enabled: root.enabled
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked()
    }
}
