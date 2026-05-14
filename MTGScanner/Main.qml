import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtMultimedia
import MTGScanner.Components
import MTGScanner.Pages
import MTGScanner.Engine

ApplicationWindow {
    id: window
    width: 1200
    height: 900
    visible: true
    title: qsTr("MTGScanner")

    property bool showDrawer: true

    SideBar {
        id: sidebar

        width: 256
        height: window.height
        edge: Qt.LeftEdge
        modal: !window.showDrawer
        interactive: !window.showDrawer
        position: window.showDrawer ? 1 : 0
        visible: window.showDrawer
        channelModel: Engine.channelsModel

        onAddChannelClicked: {
            // channelWiz.channel = Engine.createChannel()
            // channelWiz.channel.captureSession.videoOutput = channelWiz.videoOutput
            channelWiz.open()
        } 
        onDeleteChannelClicked: (id) => {
            let channel = Engine.channel(id)
            if (channel)
                deleteDialog.channelOptions = channel.options
            deleteDialog.open()
        }
    }

    Dashboard {
        anchors.fill: parent
        anchors.leftMargin: window.showDrawer ? sidebar.width : undefined

        channel: sidebar.activeChannelId !== "" ? Engine.channel(sidebar.activeChannelId) : null
    }
    
    MediaDevices {
        id: mediads
    }

    ChannelWizard {
        id: channelWiz

        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: 650
        height: 550

        closePolicy: Popup.NoAutoClose
        availableCamerasModel: Engine.availableCamerasModel

        onCurrentStepChanged: {
            if (channel === null)
                return

            // if (currentStep === 0) {
            //     if (!channel.camera.active)
            //         channel.camera.start()
            // } else {
            //     if (channel.camera.active)
            //         channel.camera.stop()
            // }
        }

        onCancelClicked: {
            // if (channel.camera.active)
            //     channel.camera.stop()

            // channel.captureSession.videoOutput = null
            Engine.destroyChannel(channel)
            channel = null
            reject()
        }

        onCreateChannelClicked: {
            // CRITICAL: Clear the device to release the system handle
            // channel.camera.stop()
            // channel.captureSession.videoOutput(null)

            Engine.addChannel(channel)
            channel = null
            accept()
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
