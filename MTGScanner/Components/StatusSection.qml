import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import MTGScanner.Engine

Pane {
    id: root

    property int status: 0 // Unknown
    property color statusColor: "red"
    property int fps: -1
    property int captureFps: -1
    property int skippedFps: -1
    property int visibleCards: -1

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

            text: Qt.enumValueToString(Engine.ChannelStatus, root.status)
            font.pixelSize: 14
            font.weight: Font.Medium
            color: root.statusColor
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

            text: "Capture FPS: " + root.captureFps
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

            text: "Skipped FPS: " + root.skippedFps
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