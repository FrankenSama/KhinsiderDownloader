import QtQuick
import QtQuick.Controls.Basic

// Slim scroll bar that blends into the dark surfaces instead of the default
// light Basic style.
ScrollBar {
    id: control

    contentItem: Rectangle {
        implicitWidth: 6
        implicitHeight: 6
        radius: 3
        color: control.pressed ? Theme.accent
            : control.hovered ? Theme.textMuted
            : Theme.border
        opacity: control.size < 1.0 ? 1 : 0
    }

    background: Rectangle {
        color: "transparent"
    }
}
