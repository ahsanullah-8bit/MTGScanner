import QtQuick

Page {
    id: dashboard
    title: qsTr("Dashboard")

    Text {
        anchors.centerIn: parent
        text: qsTr("Welcome to the MTGScanner Dashboard!")
        font.pointSize: 20
    }
}