import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import MTGScanner
import MTGScanner.Engine

Page {
    id: dashboard
    title: qsTr("Dashboard")

    MediaDevices {
        id: mediaDevices
    }

    Connections {
        target: Engine
        function onEngineLoaded (loaded) {
            for (var i = 0; i < mediaDevices.videoInputs.length; i++) {
                var device = mediaDevices.videoInputs[i]
                Engine.registerCamera(device)
            }
        }
    }

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