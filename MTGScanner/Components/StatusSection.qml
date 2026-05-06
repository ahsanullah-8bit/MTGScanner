import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import MTGScanner.Engine

Pane {
    id: root

    property var metrics

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

            text: root.metrics ? Qt.enumValueToString(Engine.ChannelStatus, root.metrics.status) : "Uknown"
            font.pixelSize: 14
            font.weight: Font.Medium
            color: root.metrics ? root.metrics.statusColor : "red"
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

            text: "FPS: " + (root.metrics ? root.metrics.fps : 0)
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

            text: "Capture FPS: " + (root.metrics ? root.metrics.captureFps : 0)
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

            text: "Skipped FPS: " + (root.metrics ? root.metrics.skippedFps : 0)
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

            text: "Cards Visible: " + (root.metrics ? root.metrics.visibleCards : 0)
            font.pixelSize: 14
            opacity: 0.5
        }

        // Spacer
        Item {
            Layout.fillWidth: true
        }
    }
}