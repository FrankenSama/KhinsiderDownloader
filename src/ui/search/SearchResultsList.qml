import QtQuick 2.15
import QtQuick.Controls.Basic

Item {
    id: root
    function hideAllResults()
    {
        root.isSearching = true;
    }

    readonly property int selectedIndex: app.searchController.searchResultVM.selectedIndex
    property bool isSearching: false
    property bool hasSearched: false
    property string errorMessage: ""

    height: 600
    width: 600

    ListView {
        id: resultsList

        anchors.fill: parent
        clip: true
        spacing: 5
        reuseItems: true
        model: app.searchController.searchResultVM
        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AlwaysOn
        }

        delegate: Item {
            width: resultsList.width
            height: 58

            Rectangle {
                id: rectangle

                property bool isSelected: index === root.selectedIndex

                color: isSelected || mouseArea.containsMouse ? "#759fc7" : "#6c98c4"
                height: parent.height
                radius: 10
                width: parent.width * 0.9
                x: isSelected ? 0 : (parent.width - width) / 2
                scale: root.isSearching ? 0 : isSelected ? 0.95 : mouseArea.pressed ? 0.90 : 1.0
                opacity: root.isSearching ? 0 : 1

                Behavior on scale {
                    NumberAnimation {
                        duration: 150
                    }
                }
                Behavior on x {
                    NumberAnimation {
                        duration: 150
                        easing.type: Easing.InOutQuad
                    }
                }
                Behavior on opacity {
                    NumberAnimation {
                        duration: 100
                        easing.type: Easing.InOutQuad
                    }
                }

                MouseArea {
                    id: mouseArea

                    anchors.fill: parent
                    hoverEnabled: true

                    onClicked: {
                        app.searchController.searchResultVM.setSelectedIndex(index);
                    }
                }

                Column {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: 12
                    anchors.rightMargin: 12
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 1

                    Text {
                        width: parent.width
                        color: "white"
                        elide: Text.ElideRight
                        font.bold: true
                        font.pointSize: 13
                        text: model.name
                    }

                    Text {
                        width: parent.width
                        color: "#d9ffffff"
                        elide: Text.ElideRight
                        font.pointSize: 10
                        text: model.details
                        visible: model.details.length > 0
                    }
                }
            }
        }
    }

    Connections {
        target: app.searchController.searchResultVM
        function onSearchStarted() {
            root.hasSearched = true;
            root.errorMessage = "";
            root.hideAllResults();
        }
    }

    Connections {
        target: app.searchController.searchResultVM
        function onSearchCompleted() {
            root.isSearching = false;
        }
    }

    Connections {
        target: app.searchController.searchResultVM
        function onSearchFailed(message) {
            root.errorMessage = message;
        }
    }

    Text {
        id: placeholder

        anchors.centerIn: parent
        z: 2
        width: parent.width * 0.8
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
        color: "#b5ffffff"
        font.pointSize: 12
        visible: resultsList.count === 0 || root.isSearching
        text: {
            if (root.isSearching) {
                return qsTr("Searching...");
            }
            if (root.errorMessage.length > 0) {
                return qsTr("Search failed: ") + root.errorMessage;
            }
            if (root.hasSearched) {
                return qsTr("No albums found. Try a different search.");
            }
            return qsTr("Search for an album to get started.");
        }
    }
}
