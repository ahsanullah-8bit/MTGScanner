import QtQuick
import QtQuick.Effects

Item {
    id: control

    property string text
    property var icon: ({ "source": "", "width": 24, "height": 24, "color": "white" })
    property font font
    property color color: MTGScanner.primaryColor
    property real spacing: 5
    property bool mirrored: false
    property int alignment: Qt.AlignCenter
    property bool expanded: true
    property real topPadding: 0
    property real leftPadding: 0
    property real rightPadding: 0
    property real bottomPadding: 0
    property int animationDuration: 200

    enum Display {
        IconOnly,
        TextOnly,
        TextBesideIcon,
        TextUnderIcon
    }
    property int display: IconLabel.TextBesideIcon

    Loader {
        id: iconLoader
        active: control.display !== IconLabel.TextOnly && control.icon.source !== ""
        sourceComponent: Item {
            implicitWidth: control.icon.width
            implicitHeight: control.icon.height

            Image {
                id: iconImage
                anchors.fill: parent
                source: control.icon.source
                sourceSize: Qt.size(control.icon.width, control.icon.height)
                visible: false   // drawn by the effect
            }

            MultiEffect {
                source: iconImage
                anchors.fill: parent
                colorization: 1.0
                colorizationColor: control.icon.color
            }
        }
        onLoaded: internal.layout()
    }

    Loader {
        id: textLoader
        active: control.display !== IconLabel.IconOnly && control.text !== ""
        sourceComponent: Text {
            id: textItem
            text: control.text
            font: control.font
            color: control.color
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
            clip: true

            readonly property real desiredWidth: implicitWidth
            width: control.expanded ? desiredWidth : 0
            opacity: control.expanded ? 1 : 0

            Behavior on width {
                NumberAnimation { duration: control.animationDuration; easing.type: Easing.OutCubic }
            }
            Behavior on opacity {
                NumberAnimation { duration: control.animationDuration }
            }
        }
        onLoaded: internal.layout()
    }

    // Helper
    QtObject {
        id: internal

        readonly property real iconW: (iconLoader.active && iconLoader.item) ? iconLoader.item.implicitWidth : 0
        readonly property real iconH: (iconLoader.active && iconLoader.item) ? iconLoader.item.implicitHeight : 0
        readonly property real textW: (textLoader.active && textLoader.item) ? textLoader.item.width : 0   // current animated width
        readonly property real textH_full: (textLoader.active && textLoader.item) ? textLoader.item.implicitHeight : 0

        readonly property real effectiveSpacing: (textW > 0 && iconW > 0) ? control.spacing : 0

        readonly property real implicitW: {
            let w = 0
            if (control.display === IconLabel.TextBesideIcon)
                w = iconW + textW + effectiveSpacing
            else
                w = Math.max(iconW, textW)
            return w + control.leftPadding + control.rightPadding
        }

        readonly property real implicitH: {
            let h = 0
            // Use the full intrinsic text height to avoid jumps, even when width is zero
            if (control.display === IconLabel.TextUnderIcon)
                h = iconH + textH_full + effectiveSpacing
            else
                h = Math.max(iconH, textH_full)
            return h + control.topPadding + control.bottomPadding
        }

        function layout() {
            // Only proceed if at least one loader item is ready
            var iconItem = (iconLoader.active && iconLoader.item) ? iconLoader.item : null
            var textItem = (textLoader.active && textLoader.item) ? textLoader.item : null

            if (!iconItem && !textItem) return

            const availW = control.width - control.leftPadding - control.rightPadding
            const availH = control.height - control.topPadding - control.bottomPadding

            const curIconW = iconItem ? iconItem.implicitWidth : 0
            const curIconH = iconItem ? iconItem.implicitHeight : 0
            const curTextW = textItem ? textItem.width : 0
            const curTextH = textItem ? textItem.implicitHeight : 0
            const effSpacing = (curTextW > 0 && curIconW > 0) ? control.spacing : 0

            let iconX, iconY, textX, textY

            if (control.display === IconLabel.TextBesideIcon) {
                const totalW = curIconW + curTextW + effSpacing
                let startX = control.leftPadding
                if (control.alignment & Qt.AlignRight)
                    startX += availW - totalW
                else if (control.alignment & Qt.AlignHCenter)
                    startX += (availW - totalW) / 2

                const baseY = control.topPadding + (availH - Math.max(curIconH, curTextH)) / 2

                if (control.mirrored) {
                    if (textItem && curTextW > 0) {
                        textX = startX
                        textY = baseY + (Math.max(curIconH, curTextH) - curTextH) / 2
                        startX += curTextW + effSpacing
                    }
                    if (iconItem) {
                        iconX = startX
                        iconY = baseY + (Math.max(curIconH, curTextH) - curIconH) / 2
                    }
                } else {
                    if (iconItem) {
                        iconX = startX
                        iconY = baseY + (Math.max(curIconH, curTextH) - curIconH) / 2
                        startX += curIconW + effSpacing
                    }
                    if (textItem && curTextW > 0) {
                        textX = startX
                        textY = baseY + (Math.max(curIconH, curTextH) - curTextH) / 2
                    }
                }
            } else if (control.display === IconLabel.TextUnderIcon) {
                const totalH = curIconH + curTextH + effSpacing
                let startY = control.topPadding
                if (control.alignment & Qt.AlignBottom)
                    startY += availH - totalH
                else if (control.alignment & Qt.AlignVCenter)
                    startY += (availH - totalH) / 2

                const baseX = control.leftPadding + (availW - Math.max(curIconW, curTextW)) / 2

                if (iconItem) {
                    iconX = baseX + (Math.max(curIconW, curTextW) - curIconW) / 2
                    iconY = startY
                    startY += curIconH + effSpacing
                }
                if (textItem && curTextW > 0) {
                    textX = baseX + (Math.max(curIconW, curTextW) - curTextW) / 2
                    textY = startY
                }
            } else {
                // IconOnly or TextOnly
                if (control.display === IconLabel.IconOnly && iconItem) {
                    iconX = control.leftPadding + (availW - curIconW) / 2
                    iconY = control.topPadding + (availH - curIconH) / 2
                } else if (control.display === IconLabel.TextOnly && textItem) {
                    textX = control.leftPadding + (availW - curTextW) / 2
                    textY = control.topPadding + (availH - curTextH) / 2
                }
            }

            // Apply positions – only if item exists
            if (iconItem) {
                iconItem.x = iconX
                iconItem.y = iconY
                iconItem.width = curIconW
                iconItem.height = curIconH
            }
            if (textItem) {
                textItem.x = textX ? textX : textItem.x
                textItem.y = textY ? textY : textItem.y
                // Text width is already bound; we only need to set height
                textItem.height = curTextH
            }
        }
    }
    
    // Implicit size bound to the internal computation
    implicitWidth: internal.implicitW
    implicitHeight: internal.implicitH

    // React to property changes
    onWidthChanged: internal.layout()
    onHeightChanged: internal.layout()
    onDisplayChanged: internal.layout()
    onSpacingChanged: internal.layout()
    onAlignmentChanged: internal.layout()
    onMirroredChanged: internal.layout()
    onExpandedChanged: internal.layout()
    onTopPaddingChanged: internal.layout()
    onLeftPaddingChanged: internal.layout()
    onRightPaddingChanged: internal.layout()
    onBottomPaddingChanged: internal.layout()

    Component.onCompleted: internal.layout()
}
