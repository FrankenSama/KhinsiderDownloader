import QtQuick 2.15
import QtQuick.Controls.Basic
import "../shared"

Item {
    id: root

    // Bubbled up to Main.qml, which owns the toast overlay.
    signal toastRequested(string message)

    // Emitted when Up is pressed on the first result, so the search box can
    // take the keyboard back.
    signal focusSearchRequested()

    function hideAllResults()
    {
        root.isSearching = true;
    }

    // Moves keyboard focus into the results so the arrow keys take over.
    function takeFocus() {
        if (resultsList.count === 0) {
            return false;
        }
        resultsList.forceActiveFocus();
        if (root.selectedIndex < 0) {
            root.stepSelection(1);
        }
        return true;
    }

    function stepSelection(delta) {
        var count = resultsList.count;
        if (count === 0) {
            return;
        }
        // Pressing Up on the first row hands the keyboard back to the search
        // box, the way a browser address bar behaves.
        if (delta < 0 && root.selectedIndex <= 0) {
            root.focusSearchRequested();
            return;
        }
        var next = root.selectedIndex < 0
            ? (delta > 0 ? 0 : count - 1)
            : Math.max(0, Math.min(count - 1, root.selectedIndex + delta));
        app.searchController.searchResultVM.setSelectedIndex(next);
        resultsList.positionViewAtIndex(next, ListView.Contain);
    }

    // Enter queues the highlighted album, matching the primary action on the
    // info panel.
    function activateSelection() {
        if (root.selectedIndex < 0) {
            return;
        }
        var name = app.searchController.addAlbumToDownloads(root.selectedIndex);
        if (name.length > 0) {
            root.toastRequested("Queued " + name + " for download");
        }
    }

    function hasFocusInList() {
        return resultsList.activeFocus;
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
        ScrollBar.vertical: WScrollBar {
            policy: ScrollBar.AlwaysOn
        }

        // Arrow keys move the highlight, Enter queues the album. Escape is left
        // unhandled so the window shortcut can return focus to the search box.
        Keys.onUpPressed: function(event) {
            root.stepSelection(-1);
            event.accepted = true;
        }
        Keys.onDownPressed: function(event) {
            root.stepSelection(1);
            event.accepted = true;
        }
        Keys.onReturnPressed: function(event) {
            root.activateSelection();
            event.accepted = true;
        }
        Keys.onEnterPressed: function(event) {
            root.activateSelection();
            event.accepted = true;
        }

        delegate: Item {
            width: resultsList.width
            height: 58

            Rectangle {
                id: rectangle

                property bool isSelected: index === root.selectedIndex

                color: isSelected ? Theme.accentSoft
                    : mouseArea.containsMouse ? Theme.surfaceHover
                    : Theme.surfaceAlt
                border.width: 1
                border.color: isSelected ? Theme.accent : Theme.borderSoft
                height: parent.height
                radius: Theme.radiusMd
                width: parent.width * 0.9
                x: isSelected ? 0 : (parent.width - width) / 2
                scale: root.isSearching ? 0 : isSelected ? 0.95 : mouseArea.pressed ? 0.90 : 1.0
                opacity: root.isSearching ? 0 : 1

                // Accent rail makes the current selection obvious at a glance.
                Rectangle {
                    width: 3
                    height: parent.height * 0.5
                    radius: 2
                    color: Theme.accent
                    anchors.left: parent.left
                    anchors.leftMargin: 5
                    anchors.verticalCenter: parent.verticalCenter
                    visible: rectangle.isSelected
                }

                Behavior on color {
                    ColorAnimation {
                        duration: 120
                    }
                }
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
                        resultsList.forceActiveFocus();
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
                        color: Theme.text
                        elide: Text.ElideRight
                        font.bold: true
                        font.pointSize: 13
                        text: model.name
                    }

                    Text {
                        width: parent.width
                        color: Theme.textMuted
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
        color: Theme.textMuted
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
