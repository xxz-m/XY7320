// Adapted from EvolveUI under the MIT license.
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic

ComboBox {
    id: root

    required property var theme
    property var selectedValue
    property string placeholderText: ""
    signal valueSelected(var value)

    implicitHeight: 40
    textRole: "text"
    valueRole: "value"
    leftPadding: 12
    rightPadding: 32
    font.pixelSize: 13

    function indexForValue(value) {
        for (let index = 0; index < count; ++index) {
            if (valueAt(index) === value)
                return index
        }
        return -1
    }

    // Best-effort text extractor for both QVariantList<QVariantMap> and
    // plain QStringList / QList<QString> models.
    function rowText(index) {
        if (index < 0 || index >= count) return ""
        const data = root.model[index]
        if (data === undefined || data === null) return ""
        if (typeof data === "string") return data
        if (typeof data === "object" && data.text !== undefined) return data.text
        return String(data)
    }

    onSelectedValueChanged: currentIndex = indexForValue(selectedValue)
    onModelChanged: currentIndex = indexForValue(selectedValue)
    onCountChanged: currentIndex = indexForValue(selectedValue)
    onActivated: valueSelected(currentValue)

    contentItem: Text {
        leftPadding: 0
        text: root.currentIndex >= 0
              ? root.rowText(root.currentIndex)
              : (root.placeholderText.length > 0
                 ? root.placeholderText
                 : (root.count === 0 ? qsTr("(无选项)") : root.rowText(0)))
        color: root.currentIndex >= 0
               ? root.theme.textColor
               : root.theme.mutedTextColor
        font: root.font
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    indicator: MaterialIcon {
        anchors.right: parent.right
        anchors.rightMargin: 12
        anchors.verticalCenter: parent.verticalCenter
        icon: root.popup.visible ? "expand_less" : "expand_more"
        color: root.theme.focusColor
        font.pixelSize: 20
    }

    background: Rectangle {
        radius: 9
        color: root.theme.elevatedColor
        border.width: root.activeFocus ? 2 : 1
        border.color: root.theme.borderFor(root.activeFocus)
    }

    popup: Popup {
        y: root.height + 5
        width: root.width
        implicitHeight: Math.min(contentItem.implicitHeight + 8, 260)
        padding: 4

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: root.popup.visible ? root.delegateModel : null
            currentIndex: root.highlightedIndex
            ScrollIndicator.vertical: ScrollIndicator {}
        }

        background: Rectangle {
            radius: 9
            color: root.theme.secondaryColor
            border.width: 1
            border.color: root.theme.borderColor
        }
    }

    delegate: ItemDelegate {
        id: delegateItem
        required property int index
        required property var modelData

        width: root.width - 8
        height: 36
        highlighted: root.highlightedIndex === delegateItem.index

        contentItem: Text {
            text: {
                const d = delegateItem.modelData
                if (d === undefined || d === null) return ""
                if (typeof d === "string") return d
                if (typeof d === "object" && d.text !== undefined) return d.text
                return String(d)
            }
            color: root.theme.textColor
            font.pixelSize: 13
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        background: Rectangle {
            radius: 7
            color: delegateItem.highlighted
                   ? Qt.rgba(root.theme.focusColor.r,
                             root.theme.focusColor.g,
                             root.theme.focusColor.b,
                             0.16)
                   : "transparent"
        }
    }
}
