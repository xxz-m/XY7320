import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic
import QtQuick.Layouts
import XY7320Host

Item {
    id: root

    required property var firmwareDialog
    required property var startDialog
    required property var toast

    readonly property bool wideLayout: width >= 980
    readonly property color mutedText: Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 0.62)
    readonly property color subtleText: Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 0.46)

    function requestStart() {
        if (firmwareUploader.portName.length === 0 || !firmwareUploader.serialOpen) {
            toast.show(qsTr("请先在 设置 → 参数配置 中选择并打开设备串口。"))
            return
        }
        if (firmwareUploader.filePath.length === 0) {
            toast.show(qsTr("请先从 U 盘固件包中选择 APP .bin 固件。"))
            return
        }
        startDialog.open()
    }

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth
        clip: true

        ColumnLayout {
            width: Math.max(parent.width, 1080)
            spacing: 16

            Item { Layout.preferredHeight: 10 }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 28
                Layout.rightMargin: 28

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    Text { text: qsTr("固件升级"); color: theme.textColor; font.pixelSize: 25; font.bold: true }
                    Text { text: qsTr("从 U 盘选择 APP 固件，并通过设置页配置的设备串口完成升级"); color: root.mutedText; font.pixelSize: 13 }
                }

                Text {
                    text: firmwareUploader.serialOpen
                          ? qsTr("设备串口：%1 / %2").arg(firmwareUploader.portName).arg(firmwareUploader.baudRate)
                          : qsTr("设备串口未连接，请前往设置配置")
                    color: firmwareUploader.serialOpen ? theme.focusColor : root.mutedText
                    font.pixelSize: 12
                }
            }

            ECard {
                Layout.fillWidth: true
                Layout.leftMargin: 28
                Layout.rightMargin: 28
                padding: 18
                radius: 10
                shadowEnabled: false
                cardColor: theme.instrumentPanelColor
                borderColor: theme.instrumentBorderColor
                borderWidth: 1

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    StepItem { title: qsTr("选择固件包"); number: "1"; active: firmwareUploader.filePath.length > 0; complete: firmwareUploader.filePath.length > 0; Layout.fillWidth: true }
                    StepLine { Layout.fillWidth: true }
                    StepItem { title: qsTr("开始升级"); number: "2"; active: firmwareUploader.busy || firmwareUploader.progress > 0; complete: firmwareUploader.progress >= 1; Layout.fillWidth: true }
                }
            }

            GridLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 28
                Layout.rightMargin: 28
                Layout.bottomMargin: 28
                columns: root.wideLayout ? 2 : 1
                columnSpacing: 16
                rowSpacing: 16

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.minimumWidth: root.wideLayout ? 620 : 0
                    Layout.alignment: Qt.AlignTop
                    spacing: 16

                    ECard {
                        Layout.fillWidth: true
                        padding: 20
                        radius: 10
                        shadowEnabled: false
                        cardColor: theme.instrumentPanelColor
                        borderColor: theme.instrumentBorderColor
                        borderWidth: 1

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 14

                            RowLayout {
                                Layout.fillWidth: true
                                SectionTitle { text: qsTr("固件包"); Layout.fillWidth: true }
                                Text { text: usbManager.usbConnected ? qsTr("U盘已连接") : qsTr("未检测到U盘"); color: usbManager.usbConnected ? theme.focusColor : root.mutedText; font.pixelSize: 12 }
                                EButton { text: qsTr("刷新"); iconCharacter: "\uf021"; size: "xs"; radius: 8; shadowEnabled: false; Layout.preferredWidth: 88; Layout.preferredHeight: 36; onClicked: usbManager.refreshDrives() }
                                EButton { text: qsTr("浏览"); iconCharacter: "\uf07c"; size: "xs"; radius: 8; shadowEnabled: false; enabled: !firmwareUploader.busy; Layout.preferredWidth: 88; Layout.preferredHeight: 36; onClicked: root.firmwareDialog.open() }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 88
                                radius: 8
                                color: theme.instrumentInsetColor
                                border.color: theme.instrumentBorderColor
                                border.width: 1
                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 14
                                    spacing: 12
                                    Text { text: "\uf1c6"; font.family: "Font Awesome 6 Free"; font.pixelSize: 30; color: theme.focusColor; Layout.preferredWidth: 38 }
                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 3
                                        Text { text: firmwareUploader.filePath.length > 0 ? firmwareUploader.fileName : qsTr("尚未选择固件"); color: theme.textColor; font.pixelSize: 14; font.bold: true; Layout.fillWidth: true; elide: Text.ElideRight }
                                        Text { text: firmwareUploader.filePath.length > 0 ? firmwareUploader.filePath : qsTr("点击下方 U 盘中的 .bin 文件加载"); color: root.mutedText; font.pixelSize: 11; Layout.fillWidth: true; elide: Text.ElideMiddle }
                                    }
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 12
                                InfoBox { label: qsTr("文件大小"); value: firmwareUploader.fileSizeText; Layout.fillWidth: true }
                                InfoBox { label: qsTr("CRC32"); value: firmwareUploader.crcHex; Layout.fillWidth: true }
                            }

                            Text { text: qsTr("U盘根目录固件"); color: root.mutedText; font.pixelSize: 12; font.bold: true }
                            ListView {
                                id: firmwareList
                                Layout.fillWidth: true
                                Layout.preferredHeight: Math.min(contentHeight, 190)
                                clip: true
                                spacing: 6
                                model: usbManager.files
                                ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
                                delegate: Rectangle {
                                    required property var modelData
                                    width: firmwareList.width
                                    height: 52
                                    radius: 7
                                    color: firmwareUploader.filePath === modelData.path
                                           ? Qt.rgba(theme.focusColor.r, theme.focusColor.g, theme.focusColor.b, 0.14)
                                           : fileMouse.containsMouse ? Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 0.05) : theme.instrumentInsetColor
                                    border.color: firmwareUploader.filePath === modelData.path ? theme.focusColor : theme.instrumentBorderColor
                                    border.width: 1
                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.leftMargin: 12
                                        anchors.rightMargin: 12
                                        spacing: 10
                                        Text { text: "\uf1c6"; font.family: "Font Awesome 6 Free"; font.pixelSize: 18; color: theme.focusColor; Layout.preferredWidth: 24 }
                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            spacing: 1
                                            Text { text: modelData.name; color: theme.textColor; font.pixelSize: 13; font.bold: true; Layout.fillWidth: true; elide: Text.ElideRight }
                                            Text { text: "%1 · %2".arg(modelData.sizeText).arg(modelData.modifiedText); color: root.mutedText; font.pixelSize: 11; Layout.fillWidth: true; elide: Text.ElideRight }
                                        }
                                    }
                                    MouseArea { id: fileMouse; anchors.fill: parent; hoverEnabled: true; cursorShape: firmwareUploader.busy ? Qt.ForbiddenCursor : Qt.PointingHandCursor; onClicked: { if (!firmwareUploader.busy) firmwareUploader.filePath = modelData.path } }
                                }
                            }
                            Text { visible: usbManager.files.length === 0; text: qsTr("未在可移动盘根目录发现 .bin 固件。") ; color: root.subtleText; font.pixelSize: 12 }
                        }
                    }

                    ECard {
                        Layout.fillWidth: true
                        padding: 20
                        radius: 10
                        shadowEnabled: false
                        cardColor: theme.instrumentPanelColor
                        borderColor: theme.instrumentBorderColor
                        borderWidth: 1
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 14
                            RowLayout {
                                Layout.fillWidth: true
                                SectionTitle { text: qsTr("升级进度"); Layout.fillWidth: true }
                                ELoader { visible: firmwareUploader.busy; running: firmwareUploader.busy; size: 20; Layout.preferredWidth: 22; Layout.preferredHeight: 22 }
                                Text { text: qsTr("%1%").arg(Math.round(firmwareUploader.progress * 100)); color: theme.focusColor; font.pixelSize: 15; font.bold: true }
                                EButton { text: qsTr("取消"); iconCharacter: "\uf00d"; size: "xs"; radius: 8; shadowEnabled: false; visible: firmwareUploader.busy; enabled: firmwareUploader.busy; Layout.preferredWidth: 88; Layout.preferredHeight: 34; onClicked: firmwareUploader.cancel() }
                            }
                            EProgressBar { value: firmwareUploader.progress; label: ""; showHeader: false; Layout.fillWidth: true; Layout.preferredHeight: 12 }
                            Text { text: firmwareUploader.status; color: root.mutedText; font.pixelSize: 12; Layout.fillWidth: true; elide: Text.ElideRight }
                            EButton { text: firmwareUploader.busy ? qsTr("升级中") : qsTr("开始升级"); iconCharacter: firmwareUploader.busy ? "\uf110" : "\uf35b"; size: "s"; radius: 8; containerColor: theme.focusColor; hoverColor: Qt.darker(theme.focusColor, 1.12); textColor: "white"; iconColor: "white"; enabled: !firmwareUploader.busy; Layout.fillWidth: true; Layout.preferredHeight: 48; onClicked: root.requestStart() }
                        }
                    }
                }

                ECard {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: 470
                    radius: 10
                    padding: 18
                    shadowEnabled: false
                    cardColor: theme.instrumentPanelColor
                    borderColor: theme.instrumentBorderColor
                    borderWidth: 1
                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 10
                        RowLayout {
                            Layout.fillWidth: true
                            SectionTitle { text: qsTr("升级日志"); Layout.fillWidth: true }
                            EButton { text: qsTr("清空"); iconCharacter: "\uf2ed"; size: "xs"; radius: 8; shadowEnabled: false; enabled: !firmwareUploader.busy; Layout.preferredWidth: 82; Layout.preferredHeight: 34; onClicked: firmwareUploader.clearLog() }
                        }
                        Text { text: qsTr("APP: 0x08010000  ·  包大小: %1 B  ·  包间隔: %2 ms  ·  擦除等待: %3 ms").arg(firmwareUploader.packetSize).arg(firmwareUploader.packetDelayMs).arg(firmwareUploader.headerDelayMs); color: root.subtleText; font.pixelSize: 11; Layout.fillWidth: true; wrapMode: Text.WordWrap }
                        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: theme.instrumentBorderColor }
                        TextArea {
                            id: logArea
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            text: firmwareUploader.logText
                            readOnly: true
                            selectByMouse: true
                            wrapMode: TextArea.Wrap
                            placeholderText: qsTr("升级日志会显示在这里。")
                            color: theme.textColor
                            font.family: "Consolas"
                            font.pixelSize: 12
                            background: Rectangle { radius: 7; color: theme.instrumentInsetColor }
                            onTextChanged: cursorPosition = length
                        }
                    }
                }
            }
        }
    }

    component SectionTitle: Text { color: theme.textColor; font.pixelSize: 15; font.bold: true; Layout.fillWidth: true }
    component StepItem: ColumnLayout {
        id: stepItem
        property string title: ""
        property string number: ""
        property bool active: false
        property bool complete: false
        spacing: 6
        Layout.alignment: Qt.AlignVCenter
        Rectangle { Layout.alignment: Qt.AlignHCenter; Layout.preferredWidth: 28; Layout.preferredHeight: 28; radius: 14; color: stepItem.complete ? theme.focusColor : (stepItem.active ? Qt.rgba(theme.focusColor.r, theme.focusColor.g, theme.focusColor.b, 0.18) : theme.secondaryColor); border.color: stepItem.active || stepItem.complete ? theme.focusColor : theme.instrumentBorderColor; border.width: 1; Text { anchors.centerIn: parent; text: stepItem.complete ? "\uf00c" : stepItem.number; color: stepItem.complete ? "white" : (stepItem.active ? theme.focusColor : root.mutedText); font.family: stepItem.complete ? "Font Awesome 6 Free" : "Arial"; font.pixelSize: stepItem.complete ? 12 : 13; font.bold: true } }
        Text { text: stepItem.title; color: stepItem.active || stepItem.complete ? theme.textColor : root.mutedText; font.pixelSize: 12; font.bold: stepItem.active || stepItem.complete; horizontalAlignment: Text.AlignHCenter; Layout.fillWidth: true }
    }
    component StepLine: Rectangle { Layout.preferredHeight: 1; Layout.alignment: Qt.AlignVCenter; color: theme.instrumentBorderColor; Layout.minimumWidth: 20 }
    component InfoBox: Rectangle {
        id: infoBox
        property string label: ""
        property string value: ""
        Layout.preferredHeight: 56
        radius: 8
        color: theme.instrumentInsetColor
        border.color: theme.instrumentBorderColor
        border.width: 1
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 3
            Text { text: infoBox.label; color: root.mutedText; font.pixelSize: 11 }
            Text { text: infoBox.value; color: theme.textColor; font.pixelSize: 13; font.bold: true; elide: Text.ElideRight; Layout.fillWidth: true }
        }
    }
}
