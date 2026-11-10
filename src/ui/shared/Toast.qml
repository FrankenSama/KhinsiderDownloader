import QtQuick 2.15

// Transient bottom-centre message used to confirm actions that otherwise
// happen silently (queueing albums, failed imports, and so on).
Rectangle {
    id: root

    property string message: ""
    property int duration: 2200

    function show(text, ms) {
        root.message = text;
        root.opacity = 1;
        hideTimer.interval = (ms === undefined ? root.duration : ms);
        hideTimer.restart();
    }

    width: Math.min(toastText.implicitWidth + 36, 460)
    height: 42
    radius: 21
    color: "#e634495e"
    border.color: "#40ffffff"
    border.width: 1
    opacity: 0
    visible: opacity > 0

    Behavior on opacity {
        NumberAnimation {
            duration: 150
            easing.type: Easing.InOutQuad
        }
    }

    Text {
        id: toastText
        anchors.centerIn: parent
        width: Math.min(implicitWidth, 424)
        text: root.message
        color: "#ffffff"
        font.pointSize: 11
        elide: Text.ElideRight
        horizontalAlignment: Text.AlignHCenter
    }

    Timer {
        id: hideTimer
        onTriggered: root.opacity = 0
    }
}
