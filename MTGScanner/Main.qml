import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MTGScanner.Components
import MTGScanner.Pages

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

            onCurrentIndexChanged: {
                mainSwipeView.currentIndex = currentIndex
            }
        }

        StackLayout {
            id: mainSwipeView
            Layout.fillWidth: true
            Layout.fillHeight: true

            Dashboard {}
            CameraConfiguration {}
            OutputWindows {}
        }
    }
}
