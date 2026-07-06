// Adapted from EvolveUI under the MIT license.
import QtQuick

QtObject {
    property bool isDark: false
    property color primaryColor: isDark ? "#171A1F" : "#F2F5F9"
    property color secondaryColor: isDark ? "#22262D" : "#FFFFFF"
    property color elevatedColor: isDark ? "#2B3038" : "#F7F9FC"
    property color textColor: isDark ? "#F4F6F8" : "#18202A"
    property color mutedTextColor: isDark ? "#A8B0BA" : "#66717E"
    property color borderColor: isDark ? "#3A414B" : "#DCE2EA"
    property color focusColor: "#00AFA0"
    property color dangerColor: "#E05D68"
    property color successColor: "#20A66A"
    property color shadowColor: isDark ? "#80000000" : "#260E2438"
    property color titleBarColor: isDark ? "#252A31" : "#FFFFFF"
    property color windowButtonHoverColor: isDark ? "#343B45" : "#EEF2F6"
    property color windowButtonPressedColor: isDark ? "#3C4450" : "#E4EAF1"

    function toggleTheme() {
        isDark = !isDark
    }

    function borderFor(focused) {
        return focused ? focusColor : borderColor
    }
}

