pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root
    required property var theme
    property var options: []
    property int currentIndex: 0
    signal selected(int index, string value)

    implicitHeight: 36
    radius: 8
    color: root.theme.elevatedColor
    border.width: 1
    border.color: root.theme.borderColor

    RowLayout {
        anchors.fill: parent
        anchors.margins: 3
        spacing: 3
        Repeater {
            model: root.options
            delegate: Rectangle {
                id: segment
                required property int index
                required property var modelData
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: 6
                color: segment.index === root.currentIndex ? root.theme.secondaryColor : "transparent"
                border.width: segment.index === root.currentIndex ? 1 : 0
                border.color: root.theme.borderColor
                Text {
                    anchors.centerIn: parent
                    text: String(segment.modelData)
                    color: segment.index === root.currentIndex ? root.theme.focusColor : root.theme.mutedTextColor
                    font.pixelSize: 11
                    font.bold: segment.index === root.currentIndex
                }
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        root.currentIndex = segment.index
                        root.selected(segment.index, String(segment.modelData))
                    }
                }
            }
        }
    }
}
