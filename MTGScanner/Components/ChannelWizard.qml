import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtMultimedia
import MTGScanner
import MTGScanner.Engine

Dialog {
    id: dialog

    property alias availableCamerasModel: cameraCombo.model
    property var channel: null
    property int currentStep: 0
    readonly property int stepCount: 3

    // property alias videoOutput: videoOutput
    onChannelChanged: console.log("Channel is" + channel)

    signal cancelClicked()
    signal createChannelClicked()

    title: "Add Scan Channel"
    modal: true
    standardButtons: Dialog.NoButton

    // Wizard page content
    Item {
        id: content
        anchors.fill: parent

        // Step 1: Camera selection
        ColumnLayout {
            id: step1
            anchors.fill: parent
            visible: currentStep === 0
            spacing: 16
            Label {
                text: "Step 1: Choose a Camera"
                font.pixelSize: 14
                font.bold: true
                color: Material.foreground
                Layout.alignment: Qt.AlignLeft
            }
            ComboBox {
                id: cameraCombo
                Layout.fillWidth: true
                textRole: "description"
                currentIndex: 0
                Material.foreground: Material.foreground
                // onCurrentTextChanged: {
                //     if (channel === null || currentIndex < 0 || currentIndex >= cameraCombo.count)
                //         return

                //     // if (channel.camera.active)
                //     //     channel.camera.stop()

                //     // var selectedCamera = cameraCombo.model[currentIndex]
                //     // channel.options.cameraDevice = selectedCamera
                //     // channel.camera.cameraDevice = selectedCamera
                //     // channel.camera.start()
                // }
            }
            // Camera Preview
            Rectangle {
                Layout.fillHeight: true
                Layout.fillWidth: true
                color: Material.background
                border.color: Material.dividerColor
                radius: 4
                Label {
                    anchors.centerIn: parent
                    text: "Camera preview (landscape)"
                    color: Material.hintTextColor
                }
                // TODO: A 16:9 aspect ratio rectangle
                Rectangle {
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
                    width: 50
                    height: 20
                    color: Material.background
                    border.color: Material.dividerColor
                    clip: true
                    Label {
                        anchors.centerIn: parent
                        text: "16:9"
                        font.pixelSize: 10
                        color: Material.hintTextColor
                    }
                }

                // VideoOutput {
                //     id: videoOutput
                //     anchors.fill: parent
                // }
            }

            // Close & Open the camera based on the page.
            // onVisibleChanged: !visible ? dialog.channel.camera.stop() : dialog.channel.camera.start()
        }

        // Step 2: Channel name & detection settings
        ColumnLayout {
            id: step2
            anchors.fill: parent
            visible: currentStep === 1
            spacing: 16
            Label {
                text: "Step 2: Channel Name & Detection Settings"
                font.pixelSize: 14
                font.bold: true
                color: Material.foreground
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            }

            TextField {
                id: nameField
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop

                placeholderText: "Channel Name"
                text: "Channel"

                // onTextEdited: {
                //     if (channel === null)
                //         return
                //     channel.options.name = text
                // }
            }
            Label {
                text: "Active Filters"
                font.pixelSize: 12
                color: Material.foreground
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            }
            Flow {
                id: filterFlow
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                spacing: 8
                Repeater {
                    model: ["Rare+", "Foil Only", "Mythic", "Creature"]
                    delegate: Button {
                        text: modelData
                        checkable: true
                        // checked: channel && channel.options.filters.indexOf(modelData) >= 0
                        flat: true
                        // Toggle filter in the array
                        // onClicked: {
                        //     let filters = dialog.channel.options.filters
                        //     if (checked) {
                        //         if (filters.indexOf(modelData) === -1)
                        //             filters.push(modelData);
                        //     } else {
                        //         var idx = filters.indexOf(modelData);
                        //         if (idx >= 0) filters.splice(idx, 1);
                        //     }
                        //     dialog.channel.options.filters = filters;
                        // }
                        Material.foreground: checked ? Material.accent : Material.hintTextColor
                        Material.background: "transparent"
                        // Component.onCompleted: {
                        //     // TODO: initialize activeFilters (Rare+ and Foil Only default checked)
                        //     if (modelData === "Rare+" || modelData === "Foil Only") {
                        //         checked = true;
                        //         if (channel !== null)
                        //             channel.options.filters.push(modelData);
                        //     }
                        // }
                    }
                }
            }
            RowLayout {
                id: detThreshLayout

                // property int threshold: channel ? Math.floor(channel.options.detectionThreshold) : 0

                Layout.fillWidth: true
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                spacing: 12

                Label {
                    text: "Detection Threshold: " + detThreshLayout.threshold
                    color: Material.foreground
                }
                Slider {
                    id: thresholdSlider


                    from: 0
                    to: 100
                    stepSize: 1
                    value: 5

                    Layout.fillWidth: true

                    // onMoved: {
                    //     if (channel)
                    //         channel.options.detectionThreshold = value
                    // }
                }
            }
            Item { Layout.fillHeight: true }
        }

        // Step 3: Output window configuration
        ColumnLayout {
            id: step3
            anchors.fill: parent
            visible: currentStep === 2
            spacing: 16
            Label {
                text: "Step 3: Configure Output Window"
                font.pixelSize: 14
                font.bold: true
                color: Material.foreground
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            }
            ColumnLayout {
                spacing: 16
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop

                TextField {
                    id: outputNameField
                    Layout.fillWidth: true
                    placeholderText: "Window Name"
                    text: "Window 1"
                    // onTextEdited: {
                    //     if (channel)
                    //         channel.options.windowName = text
                    // }
                }

                // Height x Width
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12
                    ColumnLayout {
                        Layout.fillWidth: true

                        Label { text: "X Position (px)"; color: Material.foreground }
                        SpinBox {
                            id: outputXSpin
                            from: 0
                            to: 7680
                            value: 0
                            // onValueModified: {
                            //     if (channel)
                            //         channel.options.windowGeometry.x = value
                            // }
                            editable: true
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true

                        Label { text: "Y Position (px)"; color: Material.foreground }
                        SpinBox {
                            id: outputYSpin
                            from: 0
                            to: 4320
                            value: 0
                            editable: true
                            // onValueModified: {
                            //     if (channel)
                            //         channel.options.windowGeometry.y = value
                            // }
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true

                        Label { text: "Width (px)"; color: Material.foreground }
                        SpinBox {
                            id: outputWidthSpin
                            from: 100
                            to: 3840
                            editable: true
                            value: 600
                            // onValueModified: {
                            //     if (channel)
                            //         channel.options.windowGeometry.width = value
                            // }
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true

                        Label { text: "Height (px)"; color: Material.foreground }
                        SpinBox {
                            id: outputHeightSpin
                            from: 100
                            to: 2160
                            value: 400
                            editable: true
                            // onValueModified: {
                            //     if (channel)
                            //         channel.options.windowGeometry.height = value
                            // }
                        }
                    }
                }

                Label { text: "Target Monitor"; color: Material.foreground }
                ComboBox {
                    id: monitorCombo
                    Layout.fillWidth: true
                    Layout.columnSpan: 2
                    model: ["Primary Monitor", "Secondary Monitor (Right)", "Secondary Monitor (Left)"]
                    currentIndex: 0
                    // onCurrentTextChanged: {
                    //     if (channel)
                    //         channel.options.screenName = currentText
                    // }
                }
            }
        }
    }

    // ---- Footer with navigation buttons ----
    footer: RowLayout {
        spacing: 12
        Button {
            Layout.leftMargin: 20
            Layout.bottomMargin: 5

            text: "Cancel"
            flat: true
            onClicked: dialog.cancelClicked()
        }
        Item { Layout.fillWidth: true }
        Button {
            Layout.bottomMargin: 5

            text: "← Back"
            flat: true
            enabled: currentStep > 0
            onClicked: currentStep--
        }
        Button {
            Layout.rightMargin: 20
            Layout.bottomMargin: 5

            text: currentStep < stepCount - 1 ? "Next →" : "Create Channel"
            highlighted: true
            // DialogButtonBox.buttonRole: currentStep < stepCount ? DialogButtonBox.ActionRole : DialogButtonBox.AcceptRole
            onClicked: {
                if (currentStep < stepCount - 1) { // 0 -> 1 -> 2
                    currentStep++;
                } else {
                    dialog.accept()
                    // dialog.createChannelClicked()
                }
            }
        }
    }

    onClosed: {
        currentStep = 0
    }
}
