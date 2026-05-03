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

    Material.roundedScale: Material.SmallScale

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        // Header
        Label {
            Layout.fillWidth: true
            padding: 16

            text: "SCAN CHANNELS"
            font.pixelSize: 14
            font.weight: Font.DemiBold
        }

        // Channels
        ListView {
            id: channelList
            
            Layout.fillWidth: true
            Layout.fillHeight: true

            delegate: ItemDelegate {
                id: channelDelegate
                width: ListView.view.width
                height: 60

                highlighted: index === ListView.view.currentIndex

                contentItem: Item {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 12

                    RowLayout {
                        anchors.fill: parent
                        spacing: 12

                        Rectangle {
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

                            width: 12; height: 12; radius: 6
                            color: model.online ? Material.accent
                                                    : Material.color(Material.Grey, Material.Shade400)
                        }
                        Label {
                            Layout.alignment: Qt.AlignVCenter

                            text: model.channelName
                            font: channelDelegate.font
                        }
                        Item { Layout.fillHeight: true; Layout.fillWidth: true } // spacer

                        ToolButton {
                            Layout.fillHeight: true
                            Layout.alignment: Qt.AlignVCenter

                            icon.source: "qrc:/qt/qml/MTGScanner/icons/trash-2.svg"
                            visible: channelDelegate.highlighted
                            onClicked: {
                                channelModel.remove(index);
                                if (channelModel.count === 0) {
                                    root.addChannelClicked()
                                }
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

        // Add channel button
        Button {
            Layout.fillWidth: true
            Layout.margins: 12
            Layout.preferredHeight: 60
            Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
            Material.roundedScale: Material.SmallScale 

            flat: true
            highlighted: true
            text: "Add Channel"
            icon.source: "qrc:/qt/qml/MTGScanner/icons/plus.svg"
            onClicked: root.addChannelClicked()
        }
    }
}