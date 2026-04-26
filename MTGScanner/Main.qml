import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import MTGScanner.Engine
import MTGScanner.Components

ApplicationWindow {
    width: 1280
    height: 720
    visible: true
    title: qsTr("MTGScanner")

    MTGScanner.theme: MTGScanner.Dark

    color: MTGScanner.backgroundColor

    RowLayout {
        anchors.fill: parent
        spacing: 0

        SideBar {
            id: sidebar
            Layout.fillHeight: true
            // No need to set width; it follows its own expanded/collapsed state
        }

        Item {
            id: contentArea
            Layout.fillWidth: true
            Layout.fillHeight: true

            Text {
                anchors.centerIn: parent
                text: "Main Content"
                color: MTGScanner.foregroundColor
                font.pixelSize: 28
            }
        }
    }

    // GridLayout {
    //     anchors.fill: parent
    //     columns: 2
    //     rows: 2

    //     Repeater {
    //         model: Engine.engineLoaded ? CamerasModel : 0
    //         delegate: VideoOutput {
    //             Layout.fillWidth: true
    //             Layout.fillHeight: true

    //             fillMode: VideoOutput.PreserveAspectCrop
    //             Component.onCompleted: Engine.registerCameraOutSink(model.cameraId, videoSink);
    //         }
    //     }
    // }
}
