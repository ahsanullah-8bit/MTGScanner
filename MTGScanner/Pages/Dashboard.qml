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

    property channelOptions channelOps

    onChannelOpsChanged: {
        if (channelOps.isValid()) {
            Engine.registerChannelOutSink(channelOps.id, videoCard.videoOutput.videoSink)
        }
    }

    MediaDevices {
        id: mediaDevices
    }

    background: Rectangle {
        color: MTGScanner.backgroundColor
    }

    Label {
        text: "No Channels yet. Please create one!"
        font.pixelSize: 16
        font.bold: true
        color: Material.hintTextColor
        visible: !page.channelOps || !page.channelOps.isValid()

        anchors.centerIn: parent
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        anchors.topMargin: 16
        anchors.bottomMargin: 16
        spacing: 12
        visible: page.channelOps && page.channelOps.isValid()

        // Configuration Header
        HeaderSection {
            Layout.fillWidth: true
            Layout.preferredHeight: 50

            padding: 10
            leftPadding: 20
            rightPadding: 20
            Material.elevation: 2

            channelName: page.channelOps.name
            channelRunning: false
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

                camera: page.channelOps.cameraDevice
            }

            ActiveFiltersCard {
                Layout.fillWidth: true
                Layout.fillHeight: true

                filters: ["Rare+", "Foil Only"]
            }

            OutputWindowCard {
                Layout.fillWidth: true
                Layout.fillHeight: true

                options: page.channelOps
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
        }
    }
}
