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

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            spacing: 8

            ToolButton {
                icon.name: "menu"
                onClicked: sidebar.open()
            }

            Label {
                Layout.fillWidth: true

                text: "MTGScanner"
                font.pixelSize: 20
                font.weight: Font.Medium
            }

            ToolButton {
                icon.name: "play_arrow"
                onClicked: console.log("Start All")
            }
            ToolButton {
                icon.name: "stop"
                onClicked: console.log("Stop All")
            }
        }
    }

    SideBar {
        id: sidebar

        width: 256
        height: parent.height
        edge: Qt.LeftEdge
        modal: false
        interactive: true
        channelModel: ChannelsModel

        onAddChannelClicked: channelWizardLoader.active = true
    }

    Dashboard {
        anchors.fill: parent

        channelOps: sidebar.currentChannelOptions
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
