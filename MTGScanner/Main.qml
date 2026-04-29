import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MTGScanner.Components
import MTGScanner.Pages
import MTGScanner.Engine

ApplicationWindow {
    width: 1200
    height: 900
    visible: true
    title: qsTr("MTGScanner")

    MTGScanner.theme: MTGScanner.Dark

    color: MTGScanner.backgroundColor

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            color: MTGScanner.surfaceColor
            Layout.fillWidth: true
            Layout.preferredHeight: 60

            RowLayout {
                anchors.fill: parent

                Text {
                    text: "MTGScanner"
                    font.pixelSize: 18
                    font.weight: Font.Medium
                    color: MTGScanner.surfaceTextColor
                    horizontalAlignment: Text.AlignHCenter

                    Layout.leftMargin: 10
                    Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                }
            }
        }


        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            SideBar {
                id: sidebar

                channelModel: ChannelsModel

                Layout.fillHeight: true
                Layout.preferredWidth: 220

                onAddChannelClicked: channelWizardLoader.active = true
            }

            StackView {
                id: mainStackView
                Layout.fillWidth: true
                Layout.fillHeight: true

                initialItem: configurationPage
            }
        }
    }

    Component {
        id: configurationPage

        Dashboard {
            id: dashboard
            Layout.fillWidth: true
            Layout.fillHeight: true

            channelOps: sidebar.currentChannelOptions
        }
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
}
