import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import MTGScanner.Engine

ApplicationWindow {
    width: 640
    height: 480
    visible: true
    title: qsTr("MTGScanner")

    GridLayout {
        anchors.fill: parent
        columns: 2
        rows: 2

        Repeater {
            model: Engine.engineLoaded ? CamerasModel : 0
            delegate: VideoOutput {
                Layout.fillWidth: true
                Layout.fillHeight: true

                fillMode: VideoOutput.PreserveAspectCrop
                Component.onCompleted: Engine.registerCameraOutSink(model.cameraId, videoSink);
            }
        }
    }
}
