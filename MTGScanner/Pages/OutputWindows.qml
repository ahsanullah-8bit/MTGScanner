import QtQuick
import QtQuick.Controls
import MTGScanner

Page {
    id: dashboard
    title: qsTr("Output Windows")

    Text {
        anchors.centerIn: parent
        text: qsTr("Welcome to the MTGScanner Output Windows!")
        font.pointSize: 20
        color: MTGScanner.foregroundColor
    }
}