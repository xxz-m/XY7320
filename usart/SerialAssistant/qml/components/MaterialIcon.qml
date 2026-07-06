import QtQuick

Text {
    id: root
    property alias icon: root.text
    text: "circle"
    font.family: materialSymbols.name
    font.pixelSize: 20
    font.variableAxes: ({ "FILL": 0, "wght": 400, "GRAD": 0, "opsz": 24 })
    horizontalAlignment: Text.AlignHCenter
    verticalAlignment: Text.AlignVCenter

    FontLoader {
        id: materialSymbols
        source: Qt.resolvedUrl("../assets/fonts/MaterialSymbolsRounded.ttf")
    }
}
