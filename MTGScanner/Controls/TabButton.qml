import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T
import MTGScanner // for the theme colors

T.TabButton {
    id: control

    property bool expanded: false
    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    padding: 6
    spacing: 6

    icon.width: 24
    icon.height: 24
    icon.color: !enabled ? MTGScanner.placeholderTextColor : 
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
        color: control.expanded && 
                (!control.enabled ? MTGScanner.disabledTextColor :
                 control.down || control.checked ? MTGScanner.accentColor :
                 MTGScanner.placeholderTextColor)
        
        display: control.expanded ? IconLabel.TextBesideIcon : IconLabel.IconOnly
        labelItem.opacity: control.expanded ? 1 : 0
        Behavior on labelItem.opacity { NumberAnimation { duration: 200 } }
    }

    background: Rectangle {
        implicitHeight: 40
        radius: 10
        color: control.checked ? Qt.rgba(0.545, 0.639, 0.780, 0.10)  // #8BA3C7 at 10%
           : control.down ? Qt.rgba(1,1,1,0.05)                  // white/5 for press feedback
           : "transparent" 
    }
}