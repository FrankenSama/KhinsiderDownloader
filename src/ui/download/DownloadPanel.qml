import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import "../shared"

Item {
    id: root
    //todo: show speed per album, MB/MB stop button (delete)
    width: 800
    height: 600

    // Bubbled up to Main.qml, which owns the toast overlay.
    signal toastRequested(string message)

    // The paste box takes a lot of room for an action most users only need
    // once; keep it collapsible and remember the choice for this session.
    property bool urlsExpanded: true

    Rectangle {
        id: mainWindow
        state: "normal"
        states: [
            State {
                name: "normal"
                PropertyChanges { target: leftColumn; width: mainRow.width * 0.7 }
                PropertyChanges { target: rightColumn; width: mainRow.width * 0.3; x: leftColumn.width }
            },
            State {
                name: "expanded"
                PropertyChanges { target: leftColumn; width: mainRow.width }
                PropertyChanges { target: rightColumn; width: mainRow.width * 0.3; x: mainWindow.width }
            }
        ]
        width: parent.width
        height: parent.height
        color: "#2C3E50"

        Row {
            id: mainRow
            width: parent.width
            height: parent.height

            ColumnLayout {
                id: leftColumn
                width: mainRow.width * 0.7
                height: parent.height
                spacing: 8

                Item {
                    Layout.preferredHeight: 6
                }

                // Bulk URL input
                Item {
                    id: bulkArea
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.urlsExpanded ? leftColumn.height * 0.62 : 0
                    Layout.leftMargin: 10
                    Layout.rightMargin: 10
                    visible: Layout.preferredHeight > 0
                    clip: true

                    Behavior on Layout.preferredHeight {
                        NumberAnimation {
                            duration: 180
                            easing.type: Easing.InOutQuad
                        }
                    }

                    Rectangle {
                        anchors.fill: parent
                        color: "#34495E"
                        radius: 10

                        WScrollView {
                            id: bulkScroll
                            anchors.fill: parent
                            anchors.margins: 8

                            TextArea {
                                id: textfield
                                hoverEnabled: true
                                color: "#ffffff"
                                font.pointSize: 13
                                placeholderTextColor: "#b5ffffff"
                                placeholderText: qsTr("Paste album or playlist links here, one per line")
                                width: bulkScroll.width
                                wrapMode: TextArea.Wrap
                                background: null
                            }
                        }
                    }
                }

                // Primary actions
                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 44
                    Layout.leftMargin: 10
                    Layout.rightMargin: 10

                    RowLayout {
                        anchors.fill: parent
                        spacing: 10

                        WButton {
                            Layout.preferredWidth: 140
                            Layout.fillHeight: true
                            fontSize: 12
                            label: root.urlsExpanded ? qsTr("Hide link box") : qsTr("Paste links")
                            onClicked: root.urlsExpanded = !root.urlsExpanded
                        }

                        WButton {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            fontSize: 13
                            label: qsTr("Add URLs")
                            onClicked: {
                                if (textfield.text.trim().length === 0) {
                                    root.toastRequested("Paste one or more album links first");
                                    return;
                                }
                                var added = app.downloaderController.importUrls(textfield.text);
                                if (added > 0) {
                                    textfield.text = "";
                                    root.toastRequested("Queued " + added + (added === 1 ? " album" : " albums"));
                                } else {
                                    root.toastRequested("No album links found - nothing was queued");
                                }
                            }
                        }

                        WButton {
                            Layout.preferredWidth: 130
                            Layout.fillHeight: true
                            fontSize: 13
                            label: qsTr("Cancel all")
                            onClicked: {
                                if (app.downloaderController.downloaderVM.totalSongs() === 0) {
                                    root.toastRequested("Nothing is downloading right now");
                                } else {
                                    cancelOverlay.visible = true;
                                }
                            }
                        }
                    }
                }

                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40
                    Layout.leftMargin: 10
                    Layout.rightMargin: 10

                    WButton {
                        width: parent.width * 0.55
                        height: parent.height
                        anchors.horizontalCenter: parent.horizontalCenter
                        fontSize: 12
                        label: qsTr("Open download folder")
                        onClicked: app.downloaderController.openDownloadFolder()
                    }
                }

                Item {
                    Layout.fillHeight: true
                }
            }

            Column {
                id: rightColumn
                height: parent.height
                width: parent.width

                DownloadSide {
                    width: parent.width
                    height: parent.height
                }
            }
        }
    }

    // Confirmation before cancelling everything. Cancelling is destructive:
    // partial files are removed and the queue is emptied.
    Rectangle {
        id: cancelOverlay
        anchors.fill: parent
        visible: false
        color: "#aa000000"
        z: 100

        Rectangle {
            anchors.centerIn: parent
            width: 400
            height: 160
            radius: 16
            color: "#34495e"

            Column {
                anchors.centerIn: parent
                width: parent.width * 0.85
                spacing: 18

                Text {
                    width: parent.width
                    text: qsTr("Cancel all queued and running downloads?")
                    color: "#ffffff"
                    font.pointSize: 12
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                }

                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 12

                    WButton {
                        width: 160
                        height: 36
                        fontSize: 12
                        label: qsTr("Keep downloading")
                        onClicked: cancelOverlay.visible = false
                    }

                    WButton {
                        width: 120
                        height: 36
                        fontSize: 12
                        label: qsTr("Cancel all")
                        onClicked: {
                            app.downloaderController.downloaderVM.cancelAllDownloads();
                            cancelOverlay.visible = false;
                            root.toastRequested("All downloads cancelled");
                        }
                    }
                }
            }
        }
    }
}
