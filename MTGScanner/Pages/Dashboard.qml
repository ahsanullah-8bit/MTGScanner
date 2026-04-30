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
        visible: !page.channelOps.isValid()

        anchors.centerIn: parent
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        anchors.topMargin: 16
        anchors.bottomMargin: 16
        spacing: 16
        visible: page.channelOps.isValid()

        // Configuration Header
        Rectangle {
            color: MTGScanner.surfaceColor
            radius: 12
            Layout.fillWidth: true
            Layout.preferredHeight: 50

            RowLayout {
                anchors.fill: parent
                spacing: 10

                Text {
                    text: page.channelOps.name + " - Configuration"
                    font.pixelSize: 18
                    font.weight: Font.Medium
                    color: MTGScanner.surfaceTextColor
                    horizontalAlignment: Text.AlignHCenter

                    Layout.leftMargin: 10
                    Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                }
            }
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
        Rectangle {
            radius: 12
            color: MTGScanner.surfaceColor
            Layout.fillWidth: true
            Layout.preferredHeight: 50

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 8

                // Status Indicator
                Rectangle {
                    width: 12
                    height: 12
                    color: "green"
                    radius: 6
                }

                Text {
                    text: "Status:"
                    font.pixelSize: 14
                    color: MTGScanner.placeholderTextColor
                    Layout.alignment: Qt.AlignVCenter
                }

                Text {
                    text: "Connected"
                    color: "green"
                    font.pixelSize: 14
                    font.weight: Font.Medium
                    Layout.alignment: Qt.AlignVCenter
                }

                Rectangle {
                    width: 1
                    height: 20
                    color: MTGScanner.placeholderTextColor
                    opacity: 0.3
                    Layout.alignment: Qt.AlignVCenter
                }
                
                Text {
                    text: "FPS: 30"
                    font.pixelSize: 14
                    color: MTGScanner.placeholderTextColor
                    Layout.alignment: Qt.AlignVCenter
                }

                Rectangle {
                    width: 1
                    height: 20
                    color: MTGScanner.placeholderTextColor
                    opacity: 0.3
                    Layout.alignment: Qt.AlignVCenter
                }

                Text {
                    text: "Cards Visible: 4"
                    color: MTGScanner.placeholderTextColor
                    font.pixelSize: 14
                    Layout.alignment: Qt.AlignVCenter
                }

                // Spacer
                Item {
                    Layout.fillWidth: true
                }
            }
        }
    }
}
