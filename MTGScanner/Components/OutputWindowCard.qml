import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import MTGScanner

Pane {
    id: root

    Material.elevation: 2
    padding: 20

    property channelOptions options

    ColumnLayout {
        spacing: 12
        anchors.fill: parent

        Label {
            text: "OUTPUT WINDOW"
            font.weight: Font.DemiBold
            font.pixelSize: 12
            opacity: 0.6
        }

        Label {
            text: root.options.windowName + " - " + root.options.screenName
            font.pixelSize: 14
            font.weight: Font.Medium
            color: Material.foreground
        }

        // Detail: Size & Position
        Label {
            text: root.options.windowGeometry.width + "×" + root.options.windowGeometry.height +
                    " · (" + root.options.windowGeometry.x + ", " + root.options.windowGeometry.y + ")"
            font.pixelSize: 14
            font.weight: Font.Medium
            color: Material.foreground
        }
    }
}