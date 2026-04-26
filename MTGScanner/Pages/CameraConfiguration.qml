import QtQuick
import QtQuick.Controls
import MTGScanner

Page {
    id: dashboard
    title: qsTr("Camera Configuration")

    Text {
        anchors.centerIn: parent
        text: qsTr("Welcome to the MTGScanner Camera Configuration!")
        font.pointSize: 20
        color: MTGScanner.foregroundColor
    }
}