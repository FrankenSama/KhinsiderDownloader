import QtQuick 2.12
import QtQuick.Controls.Basic
import "../shared"
import QtQuick.Layouts

Rectangle {
    width: 800
    id: mainWindow
    height: 500
    color: "#2c3e50"

    // Bubbled up to Main.qml, which owns the toast overlay.
    signal toastRequested(string message)

    function performSearch() {
        var query = textfield.text.trim();
        if (query.length === 0) {
            mainWindow.toastRequested("Type something to search for");
            return;
        }
        app.searchController.doSearch(query);
    }

    function focusSearch() {
        textfield.forceActiveFocus();
        textfield.selectAll();
    }

    function handleEscape() {
        if (textfield.text.length > 0) {
            textfield.text = "";
            return;
        }
        textfield.focus = false;
    }

    Connections {
        target: app.searchController.searchResultVM
        function onSelectedIndexChanged() {
            // Keep the info panel hidden until an album is actually selected.
            mainWindow.state = app.searchController.searchResultVM.selectedIndex >= 0
                ? "normal" : "expanded";
        }
    }
    state: "expanded"
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
    transitions: [
        Transition {
            from: "*"; to: "*"
            NumberAnimation {
                properties: "width, x"
                duration: 300
                easing.type: Easing.InOutQuad
            }
        }
    ]
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
                Layout.preferredHeight: 4
            }

            // Search box + primary action
            Item {
                id: searchRow
                Layout.fillWidth: true
                Layout.preferredHeight: 40
                Layout.leftMargin: mainRow.width * 0.05
                Layout.rightMargin: mainRow.width * 0.05

                Row {
                    id: row1
                    anchors.fill: parent
                    spacing: 10

                    Rectangle {
                        id: searchField
                        width: parent.width - searchButton.width - row1.spacing
                        height: parent.height
                        radius: 15
                        color: "#6C98C4"

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 6
                            spacing: 4

                            TextField {
                                id: textfield
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                color: "#ffffff"
                                font.pointSize: 13
                                placeholderText: qsTr("Search for a game or album...")
                                placeholderTextColor: "#b5ffffff"
                                verticalAlignment: Text.AlignVCenter
                                selectByMouse: true
                                background: null
                                onAccepted: mainWindow.performSearch()
                            }

                            // Clear the current query without leaving the field.
                            Item {
                                Layout.preferredWidth: 24
                                Layout.preferredHeight: 24
                                visible: textfield.text.length > 0

                                Text {
                                    anchors.centerIn: parent
                                    text: "\u00d7"
                                    color: "#e6ffffff"
                                    font.pointSize: 18
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        textfield.text = "";
                                        textfield.forceActiveFocus();
                                    }
                                }
                            }
                        }
                    }

                    WButton {
                        id: searchButton
                        width: 110
                        height: 40
                        label: qsTr("Search")
                        onClicked: mainWindow.performSearch()
                    }
                }
            }

            // Results summary and bulk action
            Item {
                id: resultsHeader
                Layout.fillWidth: true
                Layout.preferredHeight: visible ? 28 : 0
                Layout.leftMargin: mainRow.width * 0.05
                Layout.rightMargin: mainRow.width * 0.05
                visible: app.searchController.searchResultVM.count > 0

                RowLayout {
                    anchors.fill: parent

                    Text {
                        color: "#b5ffffff"
                        font.pointSize: 11
                        verticalAlignment: Text.AlignVCenter
                        text: qsTr("Results") + " (" + app.searchController.searchResultVM.count + ")"
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    WButton {
                        Layout.preferredWidth: 96
                        Layout.preferredHeight: 26
                        fontSize: 11
                        label: qsTr("Add all")
                        onClicked: {
                            var added = app.searchController.addAllAlbumsToDownloads();
                            if (added > 0) {
                                mainWindow.toastRequested("Queued " + added + (added === 1 ? " album" : " albums") + " for download");
                            } else {
                                mainWindow.toastRequested("Nothing to add - run a search first");
                            }
                        }
                    }
                }
            }

            SearchResultsList {
                id: searchList
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }

        // Side column
        Column {
            id: rightColumn
            height: parent.height
            width: parent.width

            Rectangle {
                id: rightPanel
                width: parent.width
                height: parent.height
                color: "#34495e"

                AlbumInfoSide {
                    width: parent.width
                    height: parent.height
                    onToastRequested: function(message) {
                        mainWindow.toastRequested(message);
                    }
                }
            }
        }
    }
}
