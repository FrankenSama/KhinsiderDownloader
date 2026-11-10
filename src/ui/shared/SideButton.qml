import QtQuick 2.15

Item {
    id: root
    width: 100
    height: 100
    property var iconFallback: "qrc:/icons/about.svg";

    // Default properties
    property alias iconSource: icon.source
    property alias label: labelText.text
    // Number shown on the corner badge; hidden when zero.
    property int badgeCount: 0
    signal clicked();
    signal loaded();
    MouseArea
    {
        anchors.fill:parent
        onClicked:
        {
            root.clicked();
        }
    }
    Column {
        width: parent.width
        height: parent.height
        spacing: 0

        Image {
            id: icon
            width:parent.width
            height: parent.height * 0.7
            source:  "../../../icons/about.svg" // default icon
            anchors.horizontalCenter: parent.horizontalCenter
            fillMode: Image.PreserveAspectFit
            onStatusChanged: {
                  if (status === Image.Error) {
                      source = iconFallback;
                  }
                  if(status === Image.Ready)
                  {
                      loaded();
                  }
            }
        }

        Text {
            id: labelText
            text: "Default" // default label
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            color: "white"
        }

    }

    Rectangle {
        id: badge

        visible: root.badgeCount > 0
        width: Math.max(16, badgeText.implicitWidth + 8)
        height: 16
        radius: height / 2
        color: "#e74c3c"
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.rightMargin: 4
        anchors.topMargin: 2

        Text {
            id: badgeText

            anchors.centerIn: parent
            text: root.badgeCount > 99 ? "99+" : root.badgeCount
            color: "#ffffff"
            font.pointSize: 8
        }
    }
}
