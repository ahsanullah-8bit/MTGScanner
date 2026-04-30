import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtMultimedia

Pane {
    id: root

    Material.elevation: 2
    padding: 20

    property cameraDevice camera

    ColumnLayout {
        spacing: 12
        anchors.fill: parent

        // Header
        Label {
            text: "CAMERA SOURCE"
            font.weight: Font.DemiBold
            font.pixelSize: 12
            opacity: 0.6
        }

        // Description
        Label {
            text: root.camera.description
            font.pixelSize: 20
            font.weight: Font.Bold
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }

        // ID
        Label {
            visible: root.camera.id !== ""
            text: root.camera.id
            font.pixelSize: 13
            opacity: 0.6
            Layout.fillWidth: true
        }
    }
}