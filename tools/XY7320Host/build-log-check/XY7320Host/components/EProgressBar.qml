import QtQuick
import QtQuick.Layouts

Item {
    id: root

    property real value: 0
    property string label: qsTr("0%")
    property bool showHeader: true
    property color trackColor: theme.secondaryColor
    property color fillColor: theme.focusColor
    property color textColor: theme.textColor

    implicitHeight: root.showHeader ? 42 : 12
    implicitWidth: 360

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        RowLayout {
            visible: root.showHeader
            Layout.fillWidth: true
            Layout.preferredHeight: 16

            Text {
                text: qsTr("升级进度")
                color: root.textColor
                font.pixelSize: 13
                Layout.fillWidth: true
                elide: Text.ElideRight
            }

            Text {
                text: root.label
                color: root.textColor
                font.pixelSize: 13
                font.bold: true
                horizontalAlignment: Text.AlignRight
                Layout.preferredWidth: 64
                elide: Text.ElideRight
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 12
            radius: 6
            color: root.trackColor
            border.color: theme.borderColor
            border.width: 1
            clip: true

            Rectangle {
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: Math.max(0, Math.min(1, root.value)) * parent.width
                radius: 6
                color: root.fillColor
                Behavior on width {
                    NumberAnimation { duration: 120; easing.type: Easing.OutCubic }
                }
            }
        }
    }
}
