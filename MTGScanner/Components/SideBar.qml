import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MTGScanner // for the theme colors
import MTGScanner.Engine

Drawer {
    id: root

    property alias currentIndex: channelList.currentIndex
    property alias channelModel: channelList.model
    property channelOptions currentChannelOptions
    signal addChannelClicked()

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        // Header
        Pane {
            Layout.fillWidth: true
            padding: 16

            Label {
                text: "SCAN CHANNELS"
                font.pixelSize: 14
                font.weight: Font.DemiBold
                color: Material.foreground
            }
        }

        // Channels
        ListView {
            id: channelList
            
            Layout.fillWidth: true
            Layout.fillHeight: true

            delegate: ItemDelegate {
                width: ListView.view.width
                highlighted: index === ListView.view.currentIndex

                contentItem: Row {
                    spacing: 12
                    Rectangle {
                        width: 12; height: 12; radius: 6
                        color: model.online ? Material.accent
                                                : Material.color(Material.Grey, Material.Shade400)
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    Label {
                        text: model.channelName
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    Item { width: 1; Layout.fillWidth: true } // spacer

                    ToolButton {
                        icon.name: "close"
                        visible: highlighted
                        onClicked: {
                            channelModel.remove(index);
                            if (channelModel.count === 0) {
                                root.addChannelClicked()
                            }
                        }
                    }
                }

                onClicked: {
                    root.currentChannelOptions = Engine.channelOptions(model.channelId)
                    channelList.currentIndex = index
                }

                Component.onCompleted: {
                    if (root.currentChannelOptions.isValid())
                        return

                    root.currentChannelOptions = Engine.channelOptions(model.channelId)
                    channelList.currentIndex = index
                }
            }
        }

        // Add Channel
        // Button {
        //     Layout.fillWidth: true
        //     Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter

        //     Material.roundedScale: Material.SmallScale 

        //     text: "Add Channel"
        //     highlighted: true

        //     onClicked: root.addChannelClicked()
        // }

        // Add channel button
        Pane {
            Layout.fillWidth: true
            padding: 12
            Button {
                text: "Add Channel"
                flat: true
                width: parent.width
                icon.name: "add"
                onClicked: console.log("Add channel (demo)")
            }
        }
    }
}