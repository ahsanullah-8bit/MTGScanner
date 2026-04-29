import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material as Mt
import QtQuick.Layouts
import QtMultimedia
import MTGScanner
import MTGScanner.Engine

Page {
    id: page
    title: "Configuration Page"

    property channelOptions channelOps

    onChannelOpsChanged: {
        if (channelOps.isValid()) {
            Engine.registerChannelOutSink(channelOps.id, videoOutput.videoSink)
        }
    }

    MediaDevices {
        id: mediaDevices
    }

    background: Rectangle {
        color: MTGScanner.backgroundColor
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        anchors.topMargin: 16
        anchors.bottomMargin: 16
        spacing: 16

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
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "black"
            radius: 12
            clip: true

            VideoOutput {
                id: videoOutput
                anchors.fill: parent
                fillMode: VideoOutput.PreserveAspectFit
                // Component.onCompleted: Engine.registerChannelOutSink(page.channelOps.id, videoSink);
            }
        }

        // Configuration Section
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: false
            Layout.preferredHeight: 130
            spacing: 10

            // Camera Source Configuration
            Rectangle {
                radius: 12
                color: MTGScanner.surfaceColor
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                    spacing: 10

                    Text {
                        text: "CAMERA SOURCE"
                        color: MTGScanner.surfaceTextColor
                        font.pixelSize: 18
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Text {
                        text: page.channelOps.cameraDevice.description
                        color: MTGScanner.foregroundColor
                        font.pixelSize: 24
                        font.bold: true
                        horizontalAlignment: Text.AlignLeft
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }
                }
            }

            // Active Filters
            Rectangle {
                radius: 12
                color: MTGScanner.surfaceColor
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                    spacing: 10

                    Text {
                        text: "ACTIVE FILTERS"
                        color: MTGScanner.surfaceTextColor
                        font.pixelSize: 18
                        horizontalAlignment: Text.AlignHCenter
                    }

                    RowLayout {
                        Button {
                            text: "Filter 1"
                        }
                        Button {
                            text: "Filter 2"
                        }
                        Button {
                            text: "Filter 3"
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }
                }
            }

            // Output Window Configuration
            Rectangle {
                radius: 12
                color: MTGScanner.surfaceColor
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                    spacing: 10

                    Text {
                        text: "OUTPUT WINDOW"
                        color: MTGScanner.surfaceTextColor
                        font.pixelSize: 18
                        horizontalAlignment: Text.AlignHCenter
                    }

                    ComboBox {
                        Layout.fillWidth: true
                        model: ["Camera 1", "Camera 2", "Camera 3"]
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }
                }
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
