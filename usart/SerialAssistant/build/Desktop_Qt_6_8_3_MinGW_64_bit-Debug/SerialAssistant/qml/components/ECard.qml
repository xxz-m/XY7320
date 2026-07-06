// Adapted from EvolveUI under the MIT license.
import QtQuick
import QtQuick.Layouts
import QtQuick.Effects

Item {
    id: root

    required property var theme
    property color cardColor: theme.secondaryColor
    property real radius: 14
    property int padding: 16
    property bool shadowEnabled: true
    default property alias content: contentLayout.data

    implicitWidth: contentLayout.implicitWidth + padding * 2
    implicitHeight: contentLayout.implicitHeight + padding * 2

    MultiEffect {
        source: background
        anchors.fill: background
        visible: root.shadowEnabled
        shadowEnabled: true
        shadowColor: root.theme.shadowColor
        shadowBlur: 0.45
        shadowVerticalOffset: 3
    }

    Rectangle {
        id: background
        anchors.fill: parent
        radius: root.radius
        color: root.cardColor
        border.width: 1
        border.color: root.theme.borderColor
    }

    ColumnLayout {
        id: contentLayout
        anchors.fill: parent
        anchors.margins: root.padding
    }
}

