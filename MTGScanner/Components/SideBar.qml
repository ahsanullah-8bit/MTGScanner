import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MTGScanner // for the theme colors
import MTGScanner.Controls
import MTGScanner.Engine

Control {
    id: root

    property int currentIndex: 0
    property alias channelModel: channelListView.model
    property channelOptions currentChannelOptions
    signal addChannelClicked()

    background: Rectangle {
        anchors.fill: parent
        color: MTGScanner.surfaceColor
    }

    contentItem: ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12

        Label {
            text: "SCAN CHANNELS"
            font.pixelSize: 12
            font.bold: true
            color: "#333"

            Layout.fillWidth: true
        }

        // // Separator
        // Rectangle {
        //     Layout.fillWidth: true
        //     implicitHeight: 1
        //     color: MTGScanner.separatorColor
        // }

        ListView {
            id: channelListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 4

            model: 4
            delegate: Button {
                width: channelListView.width
                height: 48
                background: Rectangle {
                    color: index === 0 ? "#333" : "transparent"
                    Rectangle {
                        width: 3; height: parent.height
                        color: index === 0 ? "#ff6b6b" : "transparent"
                    }
                }

                contentItem: RowLayout {
                    spacing: 10
                    Rectangle {
                        width: 12; height: 12; radius: 6
                        color: index > 2 ? "#4ecdc4" : "#ff6b6b"
                    }
                    Label {
                        text: model.channelName
                        font.pixelSize: 13
                        font.weight: Font.Medium
                        color: "white"
                        Layout.fillWidth: true
                    }

                    Label {
                        text: index
                        font.pixelSize: 11
                        color: "#aaa"
                    }
                }

                onClicked: {
                    root.currentChannelOptions = Engine.channelOptions(model.channelId)
                    channelListView.currentIndex = index
                }
            }
        }

        // Add Channel
        Button {
            text: "Add Channel"

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter

            onClicked: root.addChannelClicked()
        }
    }
}
