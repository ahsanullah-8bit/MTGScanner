import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MTGScanner // for the theme colors
import MTGScanner.Controls

Control {
    id: root

    property bool expanded: false
    readonly property int collapsedWidth: 60
    readonly property int expandedWidth: 220

    implicitWidth: expanded ? expandedWidth : collapsedWidth
    Behavior on implicitWidth { NumberAnimation { duration: 250; easing.type: Easing.OutCubic } }

    Layout.fillHeight: true

    background: Rectangle {
        anchors.fill: parent
        color: MTGScanner.surfaceColor
    }

    contentItem: ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 56
            Layout.leftMargin: 12

            Button {
                id: toggleButton
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                icon.source: "icons/menu.svg"
                icon.width: 24
                icon.height: 24
                icon.color: MTGScanner.disabledTextColor
                flat: true
                background: Rectangle {
                    color: toggleButton.hovered ? MTGScanner.hoverColor : "transparent"
                    radius: 8
                }
                onClicked: root.expanded = !root.expanded
            }
        }

        // Separator
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 1
            color: MTGScanner.separatorColor
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: 8
            Layout.rightMargin: 8
            Layout.topMargin: 8
            Layout.alignment: Qt.AlignTop
            spacing: 4

            Repeater {
                model: [
                    { name: "DASHBOARD",        icon: "qrc:/qt/qml/MTGScanner/Components/icons/dashboard.svg" },
                    { name: "CONFIGURATION",    icon: "qrc:/qt/qml/MTGScanner/Components/icons/configuration.svg" },
                    { name: "OUTPUT WINDOWS",   icon: "qrc:/qt/qml/MTGScanner/Components/icons/outputwindows.svg" }
                ]

                delegate: TabButton {
                    icon.source: modelData.icon
                    icon.width: 24
                    icon.height: 24
                    text: modelData.name
                    font.pixelSize: 12
                    font.weight: Font.DemiBold
                    expanded: root.expanded

                    spacing: 14
                    leftPadding: 12
                    rightPadding: 12
                    Layout.preferredHeight: 40
                    Layout.fillWidth: true
                }
            }

            Item {
                Layout.fillHeight: true
            }
        }
    }
}