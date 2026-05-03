import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

Pane {
    id: root

    property int channelStatus: 2
    property int visibleCards: 4
    property int fps: 30

    readonly property var statusData: [
        { name: "Initializing", color: "#95a5a6" },
        { name: "Starting",     color: "#f1c40f" },
        { name: "Running",      color: "#2ecc71" },
        { name: "Stopping",     color: "#7f8c8d" },
        { name: "Stopped",      color: "#e74c3c" },
        { name: "Errored",      color: "#34495e"}
    ]

    RowLayout {
        anchors.fill: parent
        spacing: 10

        Label {
            Layout.alignment: Qt.AlignVCenter

            text: "Status: "
            font.pixelSize: 14
            opacity: 0.5
        }

        Label {
            Layout.alignment: Qt.AlignVCenter

            text: root.statusData[root.channelStatus].name
            font.pixelSize: 14
            font.weight: Font.Medium
            color: root.statusData[root.channelStatus].color
        }

        // Separator
        Rectangle {
            Layout.alignment: Qt.AlignVCenter

            width: 1
            height: 20
            opacity: 0.3
        }
        
        Label {
            Layout.alignment: Qt.AlignVCenter

            text: "FPS: " + root.fps
            font.pixelSize: 14
            opacity: 0.5
        }

        // Separator
        Rectangle {
            Layout.alignment: Qt.AlignVCenter

            width: 1
            height: 20
            opacity: 0.3
        }

        Label {
            Layout.alignment: Qt.AlignVCenter

            text: "Cards Visible: " + root.visibleCards
            font.pixelSize: 14
            opacity: 0.5
        }

        // Spacer
        Item {
            Layout.fillWidth: true
        }
    }
}