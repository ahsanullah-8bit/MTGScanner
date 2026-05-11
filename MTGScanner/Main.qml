import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import MTGScanner.Components
import MTGScanner.Pages
import MTGScanner.Engine

ApplicationWindow {
    id: window
    width: 1200
    height: 900
    visible: true
    title: qsTr("MTGScanner")

    MTGScanner.theme: MTGScanner.Dark
    color: MTGScanner.backgroundColor

    property bool showDrawer: true

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 20
            anchors.rightMargin: 20
            spacing: 8

            Label {
                Layout.fillWidth: true

                text: "MTGScanner"
                font.pixelSize: 20
                font.weight: Font.Medium
            }

            ToolButton {
                Layout.fillHeight: true

                icon.source: "qrc:/qt/qml/MTGScanner/icons/" + (false ? "pause.svg" : "play.svg")
                opacity: 0.8
                onClicked: console.log("Start All")
            }
        }
    }

    SideBar {
        id: sidebar

        width: 256
        height: window.height - header.height
        topMargin: header.height
        edge: Qt.LeftEdge
        modal: !window.showDrawer
        interactive: !window.showDrawer
        position: window.showDrawer ? 1 : 0
        visible: window.showDrawer
        channelModel: ChannelsModel // A singleton

        onAddChannelClicked: channelWizardLoader.active = true
        onDeleteChannelClicked: (channel) => {
            deleteDialog.channelOptions = channel.options
            deleteDialog.open()
        }
    }

    Dashboard {
        anchors.fill: parent
        anchors.leftMargin: window.showDrawer ? sidebar.width : undefined

        channel: Engine.currentChannel
    }
    
    // Channel Wizard Loader
    Loader {
        id: channelWizardLoader
        active: false
        asynchronous: true
        sourceComponent: channelWizardComponent
        onLoaded: {
            item.x = (parent.width - item.width) / 2
            item.y = (parent.height - item.height) / 2
            item.open()
        }
    }
    Component {
        id: channelWizardComponent
        ChannelWizard {
            onClosed: channelWizardLoader.active = false
        }
    }

    // Delete Channel Dialog
    Dialog {
        id: deleteDialog

        property channelOptions channelOptions

        anchors.centerIn: parent

        title: "Confirm Deletion"
        modal: true
        standardButtons: Dialog.Yes | Dialog.No
        onAccepted: Engine.deleteChannel(channelOptions)

        Label {
            text: "Are you sure, you want to delete the channel?"
        }
    }
}
