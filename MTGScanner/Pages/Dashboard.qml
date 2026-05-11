import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtMultimedia
import MTGScanner
import MTGScanner.Engine
import MTGScanner.Components

Page {
    id: page
    title: "Configuration Page"

    property var channel: null
    property string prevChannelId

    onChannelChanged: {
        if (prevChannelId !== "") {
            Engine.unRegisterChannelOutSink(prevChannelId)
        }

        if (channel !== null) {
            Engine.registerChannelOutSink(channel.options.id, videoCard.videoOutput.videoSink)
            prevChannelId = channel.options.id
        }
    }

    background: Rectangle {
        color: MTGScanner.backgroundColor
    }

    Label {
        text: "No Channels yet. Please create one!"
        font.pixelSize: 16
        font.bold: true
        color: Material.hintTextColor
        visible: channel === null

        anchors.centerIn: parent
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        anchors.topMargin: 16
        anchors.bottomMargin: 16
        spacing: 12
        visible: channel !== null

        // Configuration Header
        HeaderSection {
            Layout.fillWidth: true
            Layout.preferredHeight: 50

            padding: 10
            leftPadding: 20
            rightPadding: 20
            Material.elevation: 2

            channelName: channel === null ? "" : channel.options.name
            channelRunning: channel === null ? false : channel.metrics.status === Engine.Running

            onStartChannel: Engine.startChannel(channel.options.id)
            onStopChannel: Engine.stopChannel(channel.options.id)
        }

        // Live Preview Section
        CameraPreviewCard {
            id: videoCard

            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        // Configuration Section
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: false
            Layout.preferredHeight: 130
            spacing: 10

            CameraSourceCard {
                Layout.fillWidth: true
                Layout.fillHeight: true

                cameraId: channel !== null ? channel.options.cameraDevice.id : cameraId
                description: channel !== null ? channel.options.cameraDevice.description : description
            }

            ActiveFiltersCard {
                Layout.fillWidth: true
                Layout.fillHeight: true

                filters: ["Rare+", "Foil Only"]
            }

            OutputWindowCard {
                Layout.fillWidth: true
                Layout.fillHeight: true

                winName: channel !== null ? channel.options.windowName : winName
                geometry: channel !== null ? channel.options.windowGeometry : geometry
                screenName: channel !== null ? channel.options.screenName : screenName
            }
        }

        // Status Section
        StatusSection {
            Layout.fillWidth: true
            Layout.preferredHeight: 50

            padding: 10
            leftPadding: 20
            rightPadding: 20
            Material.elevation: 2

            status: channel !== null ? channel.metrics.status : status
            statusColor: channel !== null ? channel.metrics.statusColor : statusColor
            fps: channel !== null ? channel.metrics.fps : fps
            captureFps: channel !== null ? channel.metrics.captureFps : captureFps
            skippedFps: channel !== null ? channel.metrics.skippedFps : skippedFps
            visibleCards: channel !== null ? channel.metrics.visibleCards : visibleCards
        }
    }
}
