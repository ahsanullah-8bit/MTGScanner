import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MTGScanner.Components
import MTGScanner.Pages
import MTGScanner.Engine

ApplicationWindow {
    id: window
    width: 1200
    height: 900
    visible: true
    title: qsTr("MTGScanner")

    MTGScanner.theme: MTGScanner.Dark
    color: MTGScanner.backgroundColor

    property bool showDrawer: width > 1250

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
        height: window.height - header.height
        topMargin: header.height
        edge: Qt.LeftEdge
        modal: !window.showDrawer
        interactive: !window.showDrawer
        position: window.showDrawer ? 1 : 0
        visible: window.showDrawer
        channelModel: ChannelsModel

        onAddChannelClicked: channelWizardLoader.active = true
    }

    Dashboard {
        anchors.fill: parent
        anchors.leftMargin: window.showDrawer ? sidebar.width : undefined

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
