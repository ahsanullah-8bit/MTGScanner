import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtMultimedia

Pane {
    id: root

    Material.elevation: 2
    padding: 20

    property alias videoOutput: videoOutput

    Rectangle {
        anchors.fill: parent
        
        color: "black"
        radius: 12
        clip: true

        VideoOutput {
            id: videoOutput
            anchors.fill: parent
            fillMode: VideoOutput.PreserveAspectFit
        }
    }
}