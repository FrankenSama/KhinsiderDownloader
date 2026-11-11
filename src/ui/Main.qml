import QtQuick 2.15
import QtQuick.Window 2.15
import "shared"
import "search"
import "settings"
import "download"
import "about"
import QtQuick.Dialogs
import QtQuick.Layouts
Window {
    id: window
    width: 1000
    height: 600
    minimumHeight: 600
    minimumWidth: 1000
    visible: true
    color: Theme.bg
    title: qsTr("Khinsider Downloader - QT") + (window.pendingSongs > 0
        ? "  -  " + window.downloadedSongs + "/" + window.totalSongCount
        : "")

    // Set once the user has confirmed quitting while work is still pending.
    property bool allowQuit: false

    // Mirrored from the downloader model so the title bar and the sidebar badge
    // can react to progress (the model only exposes invokable getters).
    property int totalSongCount: 0
    property int downloadedSongs: 0
    readonly property int pendingSongs: Math.max(0, totalSongCount - downloadedSongs)

    function showTab(name) {
        maincol.state = name;
        if (name === "downloadtab")
            leftPanel.currentTabButton = leftdownloadbutton;
        else if (name === "searchtab")
            leftPanel.currentTabButton = leftsearchbutton;
        else if (name === "settingstab")
            leftPanel.currentTabButton = leftsettingsbutton;
        else if (name === "abouttab")
            leftPanel.currentTabButton = leftaboutbutton;
    }

    function refreshTotals() {
        var vm = app.downloaderController.downloaderVM;
        // Capture the previous state: the queue counts as finished only when
        // it had work and now has none left.
        var hadPending = window.pendingSongs > 0;
        window.totalSongCount = vm.totalSongs();
        window.downloadedSongs = vm.totalDownloadedSongs();

        if (hadPending && window.pendingSongs === 0 && window.totalSongCount > 0) {
            toast.show(qsTr("All downloads finished"));
        }
    }

    Component.onCompleted: {
        window.width = app.settings.windowWidth;
        window.height = app.settings.windowHeight;

        // Restore the position only if it still falls on a connected screen;
        // a saved monitor may have been unplugged since the last launch.
        if (app.settings.windowPositionValid) {
            var savedX = app.settings.windowX;
            var savedY = app.settings.windowY;
            var onScreen = savedX >= Screen.virtualX
                && savedX < Screen.virtualX + Screen.width
                && savedY >= Screen.virtualY
                && savedY < Screen.virtualY + Screen.height;
            if (onScreen) {
                window.x = savedX;
                window.y = savedY;
            }
        }

        if (app.settings.windowMaximized) {
            window.showMaximized();
        }

        var tab = app.settings.lastTab;
        if (tab !== "downloadtab" && tab !== "searchtab" && tab !== "settingstab"
                && tab !== "abouttab") {
            tab = "searchtab";
        }
        // Wait for the sidebar to be laid out before moving the slider.
        Qt.callLater(function() {
            window.showTab(tab);
        });
        window.refreshTotals();
    }

    Connections {
        target: app.downloaderController.downloaderVM

        function onTotalsChanged() {
            window.refreshTotals();
        }
    }

    // Keyboard shortcuts. The sidebar buttons stay the primary navigation.
    Shortcut {
        sequence: "Ctrl+F"
        onActivated: {
            window.showTab("searchtab");
            searchPanel.focusSearch();
        }
    }
    Shortcut {
        sequence: "Ctrl+1"
        onActivated: window.showTab("searchtab")
    }
    Shortcut {
        sequence: "Ctrl+2"
        onActivated: window.showTab("downloadtab")
    }
    Shortcut {
        sequence: "Ctrl+3"
        onActivated: window.showTab("settingstab")
    }
    Shortcut {
        sequence: "Ctrl+4"
        onActivated: window.showTab("abouttab")
    }
    Shortcut {
        sequence: "F1"
        onActivated: {
            if (shortcutHelp.visible)
                shortcutHelp.close();
            else
                shortcutHelp.open();
        }
    }
    Shortcut {
        sequence: "Escape"
        onActivated: {
            if (shortcutHelp.visible) {
                shortcutHelp.close();
            } else if (quitOverlay.visible) {
                quitOverlay.visible = false;
            } else if (searchPanel.visible) {
                searchPanel.handleEscape();
            }
        }
    }

    onHeightChanged: {
        // The slider follows the active button through a binding, so nothing
        // else needs to be done when the window is resized.
    }

    onClosing: function(close) {
        app.settings.saveWindowState(window.width, window.height, window.x, window.y,
                                     window.visibility === Window.Maximized, maincol.state);

        var vm = app.downloaderController.downloaderVM;
        var hasPendingWork = vm.totalSongs() > 0 && vm.totalDownloadedSongs() < vm.totalSongs();
        if (!window.allowQuit && hasPendingWork) {
            close.accepted = false;
            quitOverlay.visible = true;
        }
    }

    Column {
        id: maincol
        state: "searchtab"
        states: [
            State {
                name: "downloadtab" //centerPanel

                PropertyChanges { target: downloadPanel; visible:true }
                PropertyChanges { target: settingsPanel; visible:false }
                PropertyChanges { target: searchPanel; visible: false}
                PropertyChanges { target: aboutPanel; visible:false }
            },
            State {
                name: "searchtab"

                PropertyChanges { target: searchPanel; visible:true }
                PropertyChanges { target: settingsPanel; visible:false }
                PropertyChanges { target: downloadPanel; visible: false}
                PropertyChanges { target: aboutPanel; visible:false }
            },
            State {
                name: "settingstab"

                PropertyChanges { target: settingsPanel; visible:true }
                PropertyChanges { target: downloadPanel; visible:false }
                PropertyChanges { target: searchPanel; visible: false}
                PropertyChanges { target: aboutPanel; visible:false }
            },
            State {
                name: "abouttab"

                PropertyChanges { target: aboutPanel; visible:true }
                PropertyChanges { target: downloadPanel; visible:false }
                PropertyChanges { target: searchPanel; visible: false}
                PropertyChanges { target: settingsPanel; visible: false}
            }
        ]
        anchors.fill: parent
        spacing: 0
        Row {
            width: parent.width
            height: parent.height * 1
            spacing: 0
            Rectangle {
                id: leftPanel
                width: 100
                Layout.minimumWidth: 100
                Layout.preferredWidth: 100
                Layout.fillHeight: true
                height: parent.height
                color: Theme.surface

                // Hairline divider between the sidebar and the content area.
                Rectangle {
                    width: 1
                    height: parent.height
                    anchors.right: parent.right
                    color: Theme.borderSoft
                }

                // The sidebar highlight tracks whichever button is active.
                property Item currentTabButton: leftdownloadbutton

                Rectangle{
                    id: slider
                    color: Theme.surfaceAlt
                    radius: Theme.radiusMd
                    x: leftPanel.currentTabButton.x + 6
                    y: leftPanel.currentTabButton.y - (leftPanel.currentTabButton.height * 0.1)
                    width: leftPanel.currentTabButton.width - 12
                    height: leftPanel.currentTabButton.height * 1.2

                    Behavior on y {
                        NumberAnimation {
                            duration: 150
                            easing.type: Easing.InOutQuad
                        }
                    }
                    Behavior on height {
                        NumberAnimation {
                            duration: 150
                            easing.type: Easing.InOutQuad
                        }
                    }
                }


                ColumnLayout {
                    id: column
                    width: parent.width
                    height: parent.height
                    spacing: 20
                    Item {
                        Layout.preferredHeight: 10
                    }

                    SideButton {
                        id: leftsearchbutton
                        Layout.preferredHeight: 40
                        Layout.alignment: Qt.AlignTop
                        iconFallback: "../../icons/search.svg"
                        iconSource: "qrc:/icons/search.svg"
                        label: "Search"
                        onClicked: {
                            window.showTab("searchtab");
                        }
                    }

                    SideButton {
                        id: leftdownloadbutton
                        Layout.preferredHeight: 40
                        Layout.alignment: Qt.AlignTop
                        iconFallback: "../../icons/dl.svg"
                        iconSource: "qrc:/icons/dl.svg"
                        label: "Download"
                        badgeCount: window.pendingSongs
                        onClicked: {
                            window.showTab("downloadtab");
                        }

                    }

                    SideButton {
                        id: leftsettingsbutton
                        Layout.preferredHeight: 40
                        Layout.alignment: Qt.AlignTop
                        iconFallback: "../../icons/settings.svg"
                        iconSource: "qrc:/icons/settings.svg"
                        label: "Settings"
                        onClicked: {
                            window.showTab("settingstab");
                        }
                    }

                    Item {
                        Layout.fillHeight: true
                    }

                    SideButton {
                        id: leftaboutbutton
                        Layout.preferredHeight: 45
                        Layout.alignment: Qt.AlignBottom
                        iconFallback: "../../icons/about.svg"
                        iconSource: "qrc:/icons/about.svg"
                        label: "About"
                        onClicked: {
                            window.showTab("abouttab");
                        }
                    }
                    Item {
                        Layout.preferredHeight: 5
                    }
                }
            }
            // Center panel
            Rectangle {
                id: centerPanel
                width: parent.width - leftPanel.width
                Layout.fillWidth: true
                Layout.fillHeight: true
                height: parent.height
                color: "transparent"
                SearchPanel
                {
                    id: searchPanel;
                    width: parent.width
                    height: parent.height
                    onToastRequested: function(message) { toast.show(message); }
                }
                SettingsPanel
                {
                    id: settingsPanel;
                    width: parent.width
                    height: parent.height
                    onToastRequested: function(message) { toast.show(message); }
                }
                DownloadPanel
                {
                    id: downloadPanel;
                    width: parent.width
                    height: parent.height
                    onToastRequested: function(message) { toast.show(message); }
                }
                AboutPanel
                {
                    id: aboutPanel;
                    width: parent.width
                    height: parent.height
                    onShortcutsRequested: shortcutHelp.open()
                }
            }
        }
    }

    Toast {
        id: toast
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 28
        z: 1000
    }

    ShortcutHelp {
        id: shortcutHelp
    }

    // Confirmation shown when the window is closed with downloads still queued.
    Rectangle {
        id: quitOverlay
        anchors.fill: parent
        visible: false
        color: Theme.scrim
        z: 900

        Rectangle {
            anchors.centerIn: parent
            width: 400
            height: 160
            radius: Theme.radiusLg
            color: Theme.surfaceAlt

            Column {
                anchors.centerIn: parent
                width: parent.width * 0.85
                spacing: 18

                Text {
                    width: parent.width
                    text: qsTr("Downloads are still running.\nQuit anyway?")
                    color: Theme.text
                    font.pointSize: 12
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                }

                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 12

                    WButton {
                        width: 150
                        height: 36
                        label: qsTr("Keep downloading")
                        onClicked: quitOverlay.visible = false
                    }

                    WButton {
                        width: 90
                        height: 36
                        label: qsTr("Quit")
                        onClicked: {
                            window.allowQuit = true;
                            window.close();
                        }
                    }
                }
            }
        }
    }
}
