import QtQuick 2.15

// Small animated switch used for boolean settings.
Item {
    id: root

    signal toggled()

    property bool checked: false

    implicitWidth: 46
    implicitHeight: 24

    Rectangle {
        id: track

        anchors.fill: parent
        radius: height / 2
        color: root.checked ? "#4caf50" : "#5d6d7e"

        Behavior on color {
            ColorAnimation {
                duration: 120
            }
        }
    }

    Rectangle {
        id: knob

        width: parent.height - 6
        height: width
        radius: width / 2
        y: 3
        x: root.checked ? parent.width - width - 3 : 3
        color: "#ffffff"

        Behavior on x {
            NumberAnimation {
                duration: 120
                easing.type: Easing.InOutQuad
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            root.checked = !root.checked;
            root.toggled();
        }
    }
}
