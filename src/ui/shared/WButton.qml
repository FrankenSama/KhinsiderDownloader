import QtQuick 2.15

// Rounded action button. The default variant is the accent (primary) action;
// `secondary` renders a neutral outlined button, `danger` a destructive one.
Rectangle {
    id: buttonRect

    signal clicked();
    property alias label: buttonlabel.text

    property alias fontSize : buttonlabel.font.pointSize

    property bool secondary: false
    property bool danger: false

    readonly property color baseColor: danger ? Theme.danger
        : secondary ? Theme.surfaceAlt
        : Theme.accent
    readonly property color hoverColor: danger ? Theme.dangerHover
        : secondary ? Theme.surfaceHover
        : Theme.accentHover
    readonly property color labelColor: (secondary || danger) ? Theme.text : Theme.onAccent

    color: baseColor
    border.width: secondary ? 1 : 0
    border.color: Theme.border
    height: 40
    radius: Theme.radiusMd
    scale: 1.0

    Behavior on color {
        ColorAnimation {
            duration: 150
        }
    }

    SequentialAnimation {
        id: shrinkAnim

        NumberAnimation {
            duration: 100
            property: "scale"
            target: buttonRect
            to: 0.96
        }
    }
    SequentialAnimation {
        id: growAnim

        NumberAnimation {
            duration: 100
            property: "scale"
            target: buttonRect
            to: 1.0
        }
    }
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true

        onEntered: buttonRect.color = buttonRect.hoverColor
        onExited: buttonRect.color = buttonRect.baseColor
        onPressed: {
            shrinkAnim.running = true;
        }
        onReleased: {
            growAnim.running = true;
        }
        onClicked:
        {
            buttonRect.clicked();
        }
    }
    Text {
        id: buttonlabel

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.verticalCenter: parent.verticalCenter
        color: buttonRect.labelColor
        elide: Text.ElideRight
        font.pointSize: 16
        horizontalAlignment: Text.AlignHCenter
        text: "Add FLAC"
        verticalAlignment: Text.AlignVCenter
    }
}
