// EPaneCard — 统一的圆角卡片基类
// 圆角 12、border 1、header 高 36；body 槽在下方。
// 用法：
//   EPaneCard { title: "..."; badge: "..."; theme: theme
//     // 子节点直接作为 body
//     RowLayout { ... }
//   }
import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

Rectangle {
    id: root

    required property var theme
    property string title: ""
    property string badge: ""
    // body 是默认子属性；外部放任何 item 都会进入此槽
    default property alias body: bodyContainer.children

    color: root.theme.secondaryColor
    radius: 12
    border.width: 1
    border.color: root.theme.borderColor

    implicitHeight: 120
    implicitWidth: 200

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Header（只放 title + badge；操作按钮由 body 自己放最上面）
        Item {
            id: headerRow
            Layout.fillWidth: true
            Layout.preferredHeight: 36
            visible: root.title.length > 0 || root.badge.length > 0

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 14
                anchors.rightMargin: 14
                spacing: 8

                Text {
                    text: root.title
                    color: root.theme.textColor
                    font.pixelSize: 13
                    font.bold: true
                    visible: text.length > 0
                }

                Rectangle {
                    visible: root.badge.length > 0
                    radius: 999
                    color: root.theme.elevatedColor
                    Layout.preferredHeight: 18
                    Layout.preferredWidth: badgeText.implicitWidth + 16
                    Text {
                        id: badgeText
                        anchors.centerIn: parent
                        text: root.badge
                        color: root.theme.mutedTextColor
                        font.pixelSize: 10
                    }
                }

                Item { Layout.fillWidth: true }
            }

            // 分隔线
            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 1
                color: root.theme.borderColor
            }
        }

        // Body 容器（外部子节点落在这里）
        Item {
            id: bodyContainer
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
