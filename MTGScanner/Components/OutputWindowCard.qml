import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import MTGScanner

Pane {
    id: root

    Material.elevation: 2
    padding: 20

    property string winName: "Window 1"
    property rect geometry
    property string screenName: "Primary 1"

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
            text: root.winName + " - " + root.screenName
            font.pixelSize: 14
            font.weight: Font.Medium
            color: Material.foreground
        }

        // Detail: Size & Position
        Label {
            text: root.geometry.width + "×" + root.geometry.height +
                    " · (" + root.geometry.x + ", " + root.geometry.y + ")"
            font.pixelSize: 14
            font.weight: Font.Medium
            color: Material.foreground
        }
    }
}