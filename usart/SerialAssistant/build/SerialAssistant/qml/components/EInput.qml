// Adapted from EvolveUI under the MIT license.
import QtQuick
import QtQuick.Controls.Basic

TextField {
    id: root

    required property var theme
    property real fieldRadius: 9

    implicitHeight: 40
    leftPadding: 12
    rightPadding: 12
    color: root.theme.textColor
    placeholderTextColor: root.theme.mutedTextColor
    selectByMouse: true
    font.pixelSize: 13

    background: Rectangle {
        radius: root.fieldRadius
        color: root.theme.elevatedColor
        border.width: root.activeFocus ? 2 : 1
        border.color: root.theme.borderFor(root.activeFocus)
        Behavior on border.color { ColorAnimation { duration: 120 } }
    }
}
