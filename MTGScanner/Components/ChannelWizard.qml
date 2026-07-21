import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtMultimedia
import MTGScanner

Dialog {
    id: control

    property Channel channel: null
    property var camerasModel: null
    property alias videoOutput: videoOutput
    property bool isCameraSelected: false
    property int currentStep: 0
    readonly property int stepCount: 3

    title: "Add Scan Channel"
    standardButtons: Dialog.NoButton

    contentItem: ColumnLayout {
        spacing: 16

        // Pages
        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: 5
            Layout.rightMargin: 5
            Layout.topMargin: 5
            currentIndex: control.currentStep

            // Step 1: Camera Selection
            Item {
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 14

                    Label {
                        Layout.alignment: Qt.AlignLeft

                        text: "Step 1: Choose a Camera"
                        font.pixelSize: 16;
                        font.bold: true
                        color: Material.foreground
                    }

                    ComboBox {
                        id: cameraSelector
                        Layout.fillWidth: true
                        visible: count > 0

                        displayText: "Select a Camera"
                        model: control.camerasModel
                        textRole: "description"
                        valueRole: "device"

                        onActivated: {
                            if (channel === null
                                || currentIndex < 0
                                || currentIndex >= count)
                                return

                            if (channel.camera.active)
                                channel.camera.stop()

                            isCameraSelected = true
                            channel.options.cameraDevice = currentValue
                            channel.camera.cameraDevice = currentValue
                            channel.camera.start()
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
                            text: cameraSelector.visible ? "Camera preview (landscape)" : "No available cameras left!"
                            color: Material.hintTextColor
                        }

                        VideoOutput {
                            id: videoOutput
                            anchors.fill: parent
                        }
                    }
                }
            }

            // Step 2: Channel name & detection settings
            Item {
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 16

                    Label {
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop

                        text: "Step 2: Channel Name & Detection Settings"
                        font.pixelSize: 16; font.bold: true
                        color: Material.foreground
                    }
                    TextField {
                        id: channelNameField
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop

                        placeholderText: "Channel Name"
                        text: channel ? channel.options.name : "Channel"

                        onTextEdited: {
                            if (channel === null)
                                return
                            channel.options.name = text
                            channel.options.windowName = text + " Output"
                        }
                    }
                    Label {
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop

                        text: "Active Filters"
                        font.pixelSize: 12
                        color: Material.foreground
                    }
                    Flow {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        spacing: 8

                        Repeater {
                            model: ["Rare+", "Foil Only", "Mythic", "Creature"]
                            delegate: Button {
                                text: modelData
                                checkable: true
                                checked: channel && channel.options.filters.indexOf(modelData) >= 0
                                flat: true
                                // Toggle filter in the array
                                onClicked: {
                                    let filters = control.channel.options.filters
                                    if (checked) {
                                        if (filters.indexOf(modelData) === -1)
                                            filters.push(modelData);
                                    } else {
                                        var idx = filters.indexOf(modelData);
                                        if (idx >= 0) filters.splice(idx, 1);
                                    }
                                    control.channel.options.filters = filters;
                                }
                                Material.foreground: checked ? Material.accent : Material.hintTextColor
                                Material.background: "transparent"
                                Component.onCompleted: {
                                    // TODO: initialize activeFilters (Rare+ and Foil Only default checked)
                                    if (modelData === "Rare+" || modelData === "Foil Only") {
                                        checked = true;
                                        if (channel !== null)
                                            channel.options.filters.push(modelData);
                                    }
                                }
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        spacing: 12

                        property int threshold: channel ? Math.floor(channel.options.detectionThreshold) : 0
                        Label {
                            text: "Detection Threshold: " + parent.threshold
                            color: Material.foreground
                        }
                        Slider {
                            Layout.fillWidth: true

                            from: 0
                            to: 100
                            stepSize: 1
                            value: parent.threshold

                            onMoved: {
                                if (channel)
                                    channel.options.detectionThreshold = value
                            }
                        }
                    }
                    Item { Layout.fillHeight: true }
                }
            }

            // Step 3: Output window configuration
            Item {
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 16

                    Label {
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop

                        text: "Step 3: Configure Output Window"
                        font.pixelSize: 16; font.bold: true
                        color: Material.foreground
                    }
                    ColumnLayout {
                        spacing: 16
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignTop

                        TextField {
                            Layout.fillWidth: true

                            placeholderText: "Window Name"
                            text: channel ? channel.options.windowName : "Window"
                            onTextEdited: {
                                if (channel)
                                    channel.options.windowName = text
                            }
                        }

                        // Height x Width
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12
                            ColumnLayout {
                                Layout.fillWidth: true

                                Label { text: "X Position (px)"; color: Material.foreground }
                                SpinBox {
                                    from: 0
                                    to: 7680
                                    value: channel ? channel.options.windowGeometry.x : 0
                                    editable: true
                                    onValueModified: {
                                        if (channel)
                                            channel.options.windowGeometry.x = value
                                    }
                                }
                            }

                            ColumnLayout {
                                Layout.fillWidth: true

                                Label { text: "Y Position (px)"; color: Material.foreground }
                                SpinBox {
                                    from: 0
                                    to: 4320
                                    value: channel ? channel.options.windowGeometry.y : 0
                                    editable: true
                                    onValueModified: {
                                        if (channel)
                                            channel.options.windowGeometry.y = value
                                    }
                                }
                            }

                            ColumnLayout {
                                Layout.fillWidth: true

                                Label { text: "Width (px)"; color: Material.foreground }
                                SpinBox {
                                    from: 100
                                    to: 3840
                                    value: channel ? channel.options.windowGeometry.width : 600
                                    editable: true
                                    onValueModified: {
                                        if (channel)
                                            channel.options.windowGeometry.width = value
                                    }
                                }
                            }

                            ColumnLayout {
                                Layout.fillWidth: true

                                Label { text: "Height (px)"; color: Material.foreground }
                                SpinBox {
                                    from: 100
                                    to: 2160
                                    value: channel ? channel.options.windowGeometry.height : 400
                                    editable: true
                                    onValueModified: {
                                        if (channel)
                                            channel.options.windowGeometry.height = value
                                    }
                                }
                            }
                        }

                        Label { text: "Target Monitor"; color: Material.foreground }
                        ComboBox {
                            Layout.fillWidth: true
                            Layout.columnSpan: 2

                            model: Application.screens // ["Primary Monitor", "Secondary Monitor (Right)", "Secondary Monitor (Left)"]
                            textRole: "name"

                            Component.onCompleted: {
                                if (channel && channel.outputWindowScreen && Application.screens) {
                                    let index = -1
                                    for (var i = 0; i < Application.screens.length; ++i) {
                                        if (Application.screens[i].name === channel.outputWindowScreen.name) {
                                            index = i
                                            break
                                        }
                                    }
                                    currentIndex = (index !== -1) ? index : 0
                                } else {
                                    currentIndex = 0
                                }
                            }

                            onActivated: (index) => {
                                if (channel && Application.screens[index])
                                    channel.setOutputWindowScreenByName(Application.screens[index].name)
                            }
                        }
                    }
                }
            }
        }
    }

    footer: RowLayout {
        spacing: 12

        Button {
            Layout.leftMargin: 20
            Layout.bottomMargin: 12

            flat: true
            text: "Cancel"
            onClicked: control.reject()
        }
        Item { Layout.fillWidth: true }
        Button {
            Layout.bottomMargin: 12

            flat: true
            text: "← Back"
            enabled: control.currentStep > 0
            onClicked: control.currentStep--
        }
        Button {
            Layout.rightMargin: 20
            Layout.bottomMargin: 12

            enabled: {
                if (control.currentStep === 0)
                    return control.camerasModel
                        && cameraSelector.count > 0
                        && cameraSelector.currentIndex >= 0
                        && isCameraSelected

                if (control.currentStep === 1)
                    return channelNameField.text !== ""

                return true
            }

            highlighted: true
            text: currentStep < stepCount - 1 ? "Next →" : "Create Channel"
            onClicked: {
                if (currentStep < stepCount - 1) { // 0 -> 1 -> 2
                    currentStep++;
                } else {
                    control.accept()
                }
            }
        }
    }
}