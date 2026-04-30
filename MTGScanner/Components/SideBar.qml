import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MTGScanner // for the theme colors
import MTGScanner.Engine

Control {
    id: root

    property alias currentIndex: channelListView.currentIndex
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

            delegate: Button {
                width: channelListView.width
                height: 48
                Material.roundedScale: Material.SmallScale

                text: model.channelName
                font.pixelSize: 13
                font.weight: Font.Medium
                flat: true
                checkable: true
                autoExclusive: true
                checked: channelListView.currentIndex === index

                onClicked: {
                    root.currentChannelOptions = Engine.channelOptions(model.channelId)
                    channelListView.currentIndex = index
                }

                Component.onCompleted: {
                    if (root.currentChannelOptions.isValid())
                        return

                    root.currentChannelOptions = Engine.channelOptions(model.channelId)
                    channelListView.currentIndex = index
                }
            }
        }

        // Add Channel
        Button {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter

            Material.roundedScale: Material.SmallScale 

            text: "Add Channel"
            highlighted: true

            onClicked: root.addChannelClicked()
        }
    }
}
