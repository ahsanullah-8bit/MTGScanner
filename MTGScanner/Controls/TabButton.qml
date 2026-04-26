import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T
import MTGScanner // for the theme colors

T.TabButton {
    id: control

    property bool expanded: false
    property int animationDuration: 200
    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    padding: 6
    spacing: 6

    icon.width: 24
    icon.height: 24
    icon.color: !enabled ? MTGScanner.disabledTextColor : 
        down || checked ? MTGScanner.accentColor : MTGScanner.placeholderTextColor

    contentItem: IconLabel {
        anchors.fill: parent
        anchors.leftMargin: control.leftPadding
        anchors.rightMargin: control.rightPadding
        anchors.topMargin: control.topPadding
        anchors.bottomMargin: control.bottomPadding
        spacing: control.spacing

        icon: control.icon
        text: control.text
        font: control.font
        color: !control.enabled ? MTGScanner.disabledTextColor :
                 control.down || control.checked ? MTGScanner.accentColor :
                 MTGScanner.placeholderTextColor
        
        expanded: control.expanded
        animationDuration: control.animationDuration
        display: IconLabel.TextBesideIcon
        alignment: Qt.AlignLeft
    }

    background: Rectangle {
        implicitHeight: 40
        radius: 10
        color: control.checked ? Qt.rgba(0.545, 0.639, 0.780, 0.10)  // #8BA3C7 at 10%
           : control.down ? Qt.rgba(1,1,1,0.05)                  // white/5 for press feedback
           : control.hovered ? MTGScanner.hoverColor : "transparent"
    }
}