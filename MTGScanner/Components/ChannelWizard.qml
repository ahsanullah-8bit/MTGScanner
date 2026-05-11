import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtMultimedia
import MTGScanner
import MTGScanner.Engine

Dialog {
    id: dialog

    property var channel: null
    property int currentStep: 0
    readonly property int stepCount: 3

    title: "Add Scan Channel"
    modal: true
    standardButtons: Dialog.NoButton

    width: 650
    height: 550
    x: parent.x + (parent.width / 2 - width / 2)
    y: parent.y + (parent.height / 2 - height / 2)

    Component.onCompleted: {
        dialog.channel = Engine.createChannel()
        dialog.channel.captureSession.setVideoOutput(videoOutput)
    }

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
                model: Engine.availableCameras
                currentIndex: 0
                Material.foreground: Material.foreground
                onCurrentTextChanged: {
                    if (dialog.channel.camera.active)
                        dialog.channel.camera.stop()

                    var selectedCamera = cameraCombo.model[currentIndex]
                    dialog.channel.options.cameraDevice = selectedCamera
                    dialog.channel.camera.cameraDevice = selectedCamera
                    dialog.channel.camera.start()
                }
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

                VideoOutput {
                    id: videoOutput
                    anchors.fill: parent
                }
            }

            // Close & Open the camera based on the page.
            onVisibleChanged: !visible ? dialog.channel.camera.stop() : dialog.channel.camera.start()
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
                text: dialog.channel.options.name

                onTextEdited: dialog.channel.options.name = text
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
                        checked: dialog.channel.options.filters.indexOf(modelData) >= 0
                        flat: true
                        // Toggle filter in the array
                        onClicked: {
                            let filters = dialog.channel.options.filters
                            if (checked) {
                                if (filters.indexOf(modelData) === -1)
                                    filters.push(modelData);
                            } else {
                                var idx = filters.indexOf(modelData);
                                if (idx >= 0) filters.splice(idx, 1);
                            }
                            dialog.channel.options.filters = filters;
                        }
                        Material.foreground: checked ? Material.accent : Material.hintTextColor
                        Material.background: "transparent"
                        Component.onCompleted: {
                            // TODO: initialize activeFilters (Rare+ and Foil Only default checked)
                            if (modelData === "Rare+" || modelData === "Foil Only") {
                                checked = true;
                                dialog.channel.options.filters.push(modelData);
                            }
                        }
                    }
                }
            }
            RowLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                spacing: 12

                property int threshold: Math.floor(dialog.channel.options.detectionThreshold)
                Label {
                    text: "Detection Threshold: " + parent.threshold
                    color: Material.foreground
                }
                Slider {
                    id: thresholdSlider
                    from: 0
                    to: 100
                    stepSize: 1
                    value: parent.threshold
                    onMoved: dialog.channel.options.detectionThreshold = value
                    Layout.fillWidth: true
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
                    text: dialog.channel.options.windowName
                    onTextEdited: dialog.channel.options.windowName = text
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
                            value: dialog.channel.options.windowGeometry.x
                            onValueModified: dialog.channel.options.windowGeometry.x = value
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
                            value: dialog.channel.options.windowGeometry.y
                            onValueModified: dialog.channel.options.windowGeometry.y = value
                            editable: true
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true

                        Label { text: "Width (px)"; color: Material.foreground }
                        SpinBox {
                            id: outputWidthSpin
                            from: 100
                            to: 3840
                            value: dialog.channel.options.windowGeometry.width
                            onValueModified: dialog.channel.options.windowGeometry.width = value
                            editable: true
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true

                        Label { text: "Height (px)"; color: Material.foreground }
                        SpinBox {
                            id: outputHeightSpin
                            from: 100
                            to: 2160
                            value: dialog.channel.options.windowGeometry.height
                            onValueModified: dialog.channel.options.windowGeometry.height = value
                            editable: true
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
                    onCurrentTextChanged: dialog.channel.options.screenName = currentText
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
            onClicked: {
                dialog.channel.camera.stop()
                dialog.channel.captureSession.setVideoOutput(null)
                Engine.destroyChannel(dialog.channel)
                dialog.channel = null
                dialog.reject()
            }
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
                    // CRITICAL: Clear the device to release the system handle
                    dialog.channel.camera.stop()
                    dialog.channel.captureSession.setVideoOutput(null)

                    Engine.addChannel(dialog.channel)
                    dialog.accept();
                }
            }
        }
    }

    onClosed: {
        currentStep = 0
    }
}
