// EOptRow — 复选框行：左侧 22×22 自绘 CheckBox + 右侧文字标签
// 整行可点击切换；hover 高亮；禁用时变灰。
import QtQuick
import QtQuick.Controls.Basic

Rectangle {
    id: optRow

    required property var theme
    property string label: ""
    property bool checked: false
    property bool enabled: true

    implicitHeight: 28
    radius: 5
    color: optHover.hovered && enabled ? optRow.theme.elevatedColor : "transparent"

    function toggle() {
        if (enabled) optRow.checked = !optRow.checked
    }

    // 横向布局：左 CheckBox + 文字标签
    Row {
        anchors.fill: parent
        anchors.leftMargin: 4
        anchors.rightMargin: 4
        spacing: 6

        CheckBox {
            id: optCheck
            width: 22
            height: 22
            anchors.verticalCenter: parent.verticalCenter
            checked: optRow.checked
            enabled: optRow.enabled
            onToggled: optRow.checked = checked

            // 缩到目标大小，防止 indicator 被父布局拉伸
            indicator: Rectangle {
                implicitWidth: 16
                implicitHeight: 16
                x: (parent.width - width) / 2
                y: (parent.height - height) / 2
                radius: 3
                color: optCheck.checked ? optRow.theme.focusColor : "transparent"
                border.color: optCheck.enabled
                               ? (optCheck.checked
                                  ? optRow.theme.focusColor
                                  : optRow.theme.borderColor)
                               : optRow.theme.borderColor
                border.width: 1
                MaterialIcon {
                    anchors.centerIn: parent
                    visible: optCheck.checked
                    icon: "check"
                    color: "white"
                    font.pixelSize: 14
                }
            }
            contentItem: Item {}  // 不显示 CheckBox 自带文字
        }

        Text {
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width - optCheck.width - 6
            text: optRow.label
            color: optRow.enabled
                   ? (optRow.checked ? optRow.theme.focusColor : optRow.theme.textColor)
                   : optRow.theme.mutedTextColor
            font.pixelSize: 12
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }
    }

    // 整行点击切换：TapHandler 不阻挡 CheckBox
    TapHandler {
        id: optHover
        enabled: optRow.enabled
        onTapped: optRow.toggle()
    }
    HoverHandler {
        enabled: optRow.enabled
        cursorShape: optRow.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
    }
}
