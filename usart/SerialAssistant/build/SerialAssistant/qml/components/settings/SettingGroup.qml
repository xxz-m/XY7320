import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root
    required property var theme
    property string title: ""
    property string description: ""
    default property alias content: contentColumn.children

    implicitHeight: groupLayout.implicitHeight + 28
    radius: 10
    color: root.theme.isDark ? root.theme.elevatedColor : "#FBFCFE"
    border.width: 1
    border.color: root.theme.borderColor

    ColumnLayout {
        id: groupLayout
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 14
        spacing: 4

        Text {
            text: root.title
            color: root.theme.textColor
            font.pixelSize: 13
            font.bold: true
            visible: text.length > 0
        }
        Text {
            Layout.fillWidth: true
            text: root.description
            color: root.theme.mutedTextColor
            font.pixelSize: 11
            wrapMode: Text.Wrap
            visible: text.length > 0
            Layout.bottomMargin: visible ? 8 : 0
        }
        ColumnLayout {
            id: contentColumn
            Layout.fillWidth: true
            spacing: 8
        }
    }
}
