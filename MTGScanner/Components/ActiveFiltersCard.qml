import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

Pane {
    id: root

    Material.elevation: 2
    padding: 20

    property var filters: []

    // Layout
    ColumnLayout {
        spacing: 12
        anchors.fill: parent

        Label {
            text: "ACTIVE FILTERS"
            font.weight: Font.DemiBold
            font.pixelSize: 12
            color: Material.foreground
            opacity: 0.6
        }

        Flow {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 8

            Repeater {
                model: filters.length > 0 ? filters : []
                delegate: tagDelegate
            }
        }

        // Empty state
        Label {
            visible: filters.length === 0
            text: "No filters applied"
            font.italic: true
            font.pixelSize: 12
            color: Material.foreground
            opacity: 0.3
        }
    }

    Component {
        id: tagDelegate

        Rectangle {
            id: tag
            width: tagRow.implicitWidth + 16   // 8px padding on each side
            height: tagRow.implicitHeight + 12
            radius: height / 2

            color: Qt.rgba(Material.accent.r, Material.accent.g, Material.accent.b, 0.08)
            border {
                color: Qt.rgba(Material.accent.r, Material.accent.g, Material.accent.b, 0.3)
                width: 1
            }

            Row {
                id: tagRow
                spacing: 6
                anchors.centerIn: parent

                // Filter name
                Label {
                    text: modelData
                    font.pixelSize: 13
                    font.weight: Font.Medium
                    color: Material.accent
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }
    }
}