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

    SideBar {
        id: sidebar

        width: 256
        height: window.height
        edge: Qt.LeftEdge
        modal: false
        interactive: false
        position: 1
        visible: true
        channelModel: Engine.channelsModel

        onAddChannelClicked: wizLoader.active = true
        onDeleteChannelClicked: (id) => {
            let channel = Engine.channel(id)
            if (channel) {
                deleteDialog.channelOptions = channel.options
                deleteDialog.open()
            }
        }
    }

    Dashboard {
        anchors.fill: parent
        anchors.leftMargin: sidebar.width

        channel: sidebar.activeChannelId !== "" ? Engine.channel(sidebar.activeChannelId) : null
    }
    
    Loader {
        id: wizLoader

        active: false
        onLoaded: {
            item.width = 650
            item.height = 550
            item.channel = Engine.createChannel()
            item.camerasModel = Engine.availableCamerasModel
            item.channel.captureSession.videoOutput = item.videoOutput
            item.open()
        }

        sourceComponent: ChannelWizard {
            x: (window.width - width) / 2
            y: (window.height - height) / 2
            closePolicy: Popup.NoAutoClose

            onCurrentStepChanged: {
                if (channel === null)
                    return

                if (currentStep === 0) {
                    if (!channel.camera.active)
                        channel.camera.start()
                } else {
                    if (channel.camera.active)
                        channel.camera.stop()
                }
            }

            onRejected: {
                if (channel.camera.active)
                    channel.camera.stop()

                channel.captureSession.videoOutput = null
                Engine.destroyChannel(channel)
            }

            onAccepted: {
                if (channel.camera.active)
                    channel.camera.stop()
                    
                channel.captureSession.videoOutput = null
                Engine.addChannel(channel)
            }

            onClosed: {
                channel = null
                currentStep = 0
                wizLoader.active = false
            }
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
