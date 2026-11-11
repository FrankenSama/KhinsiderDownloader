import QtQuick
import QtQuick.Layouts

// In-app shortcut reference. Opened with F1 or from the About page, closed
// with Escape, the Close button, or a click on the dimmed background.
Rectangle {
    id: root

    anchors.fill: parent
    visible: false
    color: Theme.scrim
    z: 1100

    function open() {
        root.visible = true;
    }

    function close() {
        root.visible = false;
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.close()
    }

    Rectangle {
        anchors.centerIn: parent
        width: Math.min(parent.width - 80, 520)
        implicitHeight: content.implicitHeight + 40
        radius: Theme.radiusLg
        color: Theme.surfaceAlt
        border.width: 1
        border.color: Theme.border

        // Keep clicks on the card from reaching the background MouseArea.
        MouseArea {
            anchors.fill: parent
        }

        ColumnLayout {
            id: content

            anchors.fill: parent
            anchors.margins: 20
            spacing: 10

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Text {
                    Layout.fillWidth: true
                    color: Theme.text
                    font.pointSize: 16
                    font.bold: true
                    text: qsTr("Keyboard shortcuts")
                    verticalAlignment: Text.AlignVCenter
                }

                WButton {
                    Layout.preferredWidth: 90
                    Layout.preferredHeight: 32
                    fontSize: 12
                    secondary: true
                    label: qsTr("Close")
                    onClicked: root.close()
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: Theme.borderSoft
            }

            Repeater {
                model: [
                    { keys: "Ctrl+F", action: qsTr("Focus the search box") },
                    { keys: "Ctrl+1", action: qsTr("Open the Search tab") },
                    { keys: "Ctrl+2", action: qsTr("Open the Download tab") },
                    { keys: "Ctrl+3", action: qsTr("Open the Settings tab") },
                    { keys: "Ctrl+4", action: qsTr("Open the About tab") },
                    { keys: "Enter", action: qsTr("Queue the selected search result") },
                    { keys: "Esc", action: qsTr("Close dialogs or clear the search box") },
                    { keys: "F1", action: qsTr("Show or hide this help") }
                ]

                delegate: RowLayout {
                    Layout.fillWidth: true
                    spacing: 14

                    Rectangle {
                        Layout.preferredWidth: 92
                        Layout.preferredHeight: 24
                        radius: Theme.radiusSm
                        color: Theme.surface
                        border.width: 1
                        border.color: Theme.borderSoft

                        Text {
                            anchors.centerIn: parent
                            color: Theme.accent
                            font.pointSize: 10
                            font.bold: true
                            text: modelData.keys
                        }
                    }

                    Text {
                        Layout.fillWidth: true
                        color: Theme.text
                        font.pointSize: 11
                        wrapMode: Text.WordWrap
                        text: modelData.action
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
    }
}
