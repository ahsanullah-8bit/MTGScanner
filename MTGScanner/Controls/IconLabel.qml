import QtQuick
import QtQuick.Effects

Item {
    id: control
    
    property string text
    property var icon: { "source": "", "width": 24, "height": 24, "color": "white" }
    property font font
    property color color: MTGScanner.primaryColor
    property real spacing: 5
    property bool mirrored: false
    property int alignment: Qt.AlignCenter
    
    property real topPadding: 0
    property real leftPadding: 0
    property real rightPadding: 0
    property real bottomPadding: 0

    property alias labelItem: textItem
    property alias iconItem: iconItem

    enum Display {
        IconOnly,
        TextOnly,
        TextBesideIcon,
        TextUnderIcon
    }
    property int display: IconLabel.TextBesideIcon

    implicitWidth: layout.implicitWidth + leftPadding + rightPadding
    implicitHeight: layout.implicitHeight + topPadding + bottomPadding

    Grid {
        id: layout
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: leftPadding
        anchors.topMargin: topPadding
        anchors.rightMargin: rightPadding
        anchors.bottomMargin: bottomPadding
        
        spacing: control.spacing
        columns: display === IconLabel.TextUnderIcon ? 1 : 2
        rows: display === IconLabel.TextUnderIcon ? 2 : 1
        layoutDirection: mirrored ? Qt.RightToLeft : Qt.LeftToRight
        horizontalItemAlignment: Grid.AlignHCenter
        verticalItemAlignment: Grid.AlignVCenter

        Item {
            id: iconContainer
            implicitWidth: control.icon.width
            implicitHeight: control.icon.height
            visible: display !== IconLabel.TextOnly && iconItem.source != ""

            Image {
                id: iconItem
                anchors.fill: parent
                source: control.icon.source
                sourceSize: Qt.size(control.icon.width, control.icon.height)
                visible: false // Hidden because MultiEffect draws it
            }

            MultiEffect {
                source: iconItem
                anchors.fill: parent
                colorization: 1.0
                colorizationColor: control.icon.color
            }
        }

        Text {
            id: textItem
            text: control.text
            font: control.font
            color: control.color
            visible: display !== IconLabel.IconOnly && text !== ""
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }
}
