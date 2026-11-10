import QtQuick
import QtQuick.Dialogs
import "../shared"
import QtQuick.Layouts
import QtQuick.Controls.Basic

Rectangle {
    id: root
    color: "#2c3e50"
    height: 700
    width: 400

    function openLogFolder() {
        var dir = app.logController.logDir;
        if (Qt.platform.os === "windows") {
            Qt.openUrlExternally("file:///" + dir);
        } else {
            Qt.openUrlExternally("file://" + dir);
        }
    }

    FolderDialog {
        id: folderDialog

        currentFolder: app.settings.downloadPath
        selectedFolder: app.settings.downloadPath

        onAccepted: {
            app.settings.setDownloadPath(selectedFolder);
        }
    }

    WScrollView {
        id: scroll
        anchors.fill: parent
        anchors.margins: 12
        // Leave room for the always-visible scroll bar so the right-hand
        // controls are not drawn underneath it.
        rightPadding: 18
        clip: true

        ColumnLayout {
            id: maincolumn
            width: scroll.availableWidth
            spacing: 10

            Text {
                Layout.topMargin: 4
                color: "#b5ffffff"
                font.pointSize: 11
                font.bold: true
                text: qsTr("DOWNLOADS")
            }

            // Download folder
            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40
                    radius: 10
                    color: "#6c98c4"

                    Text {
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.rightMargin: 10
                        color: "#ffffff"
                        font.pointSize: 12
                        text: app.settings.downloadPath
                        elide: Text.ElideMiddle
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                WButton {
                    Layout.preferredWidth: 110
                    Layout.preferredHeight: 40
                    fontSize: 12
                    label: qsTr("Open folder")
                    onClicked: app.downloaderController.openDownloadFolder()
                }

                WButton {
                    Layout.preferredWidth: 100
                    Layout.preferredHeight: 40
                    fontSize: 12
                    label: qsTr("Change")
                    onClicked: folderDialog.open()
                }
            }

            // Threads
            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Text {
                    Layout.fillWidth: true
                    color: "#ffffff"
                    font.pointSize: 12
                    text: qsTr("Download threads")
                    verticalAlignment: Text.AlignVCenter
                }

                WNumberBox {
                    Layout.preferredWidth: 120
                    Layout.preferredHeight: 40
                    currentNumber: app.settings.downloadThreads
                    nextNumber: app.settings.downloadThreads
                    onValueChanged: {
                        app.settings.setDownloadThreads(currentNumber);
                    }
                    maxNumber: 64
                    minNumber: 1
                }
            }

            // Concurrent downloads
            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Text {
                    Layout.fillWidth: true
                    color: "#ffffff"
                    font.pointSize: 12
                    text: qsTr("Concurrent downloads per thread")
                    verticalAlignment: Text.AlignVCenter
                }

                Text {
                    color: "#99ffffff"
                    font.pointSize: 11
                    text: qsTr("0 = no limit")
                    verticalAlignment: Text.AlignVCenter
                }

                WNumberBox {
                    Layout.preferredWidth: 120
                    Layout.preferredHeight: 40
                    currentNumber: app.settings.maxConcurrentDownloadsPerThread
                    nextNumber: app.settings.maxConcurrentDownloadsPerThread
                    onValueChanged: {
                        app.settings.setMaxConcurrentDownloadsPerThread(currentNumber);
                    }
                    maxNumber: 256
                    minNumber: 0
                }
            }

            // Audio quality
            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Text {
                    Layout.fillWidth: true
                    color: "#ffffff"
                    font.pointSize: 12
                    text: qsTr("Preferred audio quality")
                    verticalAlignment: Text.AlignVCenter
                }

                WEnumButton {
                    Layout.preferredWidth: 120
                    Layout.preferredHeight: 40
                    fontSize: 13
                    onValueChanged: {
                        app.settings.setPreferredAudioQualityInt(selectedIndex);
                    }
                    selectedIndex: app.settings.preferredAudioQuality;
                    Component.onCompleted: {
                        resetModel(["MP3", "Best"], app.settings.preferredAudioQuality);
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                Layout.topMargin: 6
                Layout.bottomMargin: 6
                color: "#33ffffff"
            }

            Text {
                color: "#b5ffffff"
                font.pointSize: 11
                font.bold: true
                text: qsTr("BEHAVIOUR")
            }

            // Download art
            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Text {
                    Layout.fillWidth: true
                    color: "#ffffff"
                    font.pointSize: 12
                    text: qsTr("Download art covers")
                    verticalAlignment: Text.AlignVCenter
                }

                WToggle {
                    Layout.preferredWidth: 46
                    Layout.preferredHeight: 24
                    checked: app.settings.downloadArt
                    onToggled: app.settings.setDownloadArt(checked)
                }
            }

            // Skip downloaded
            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Text {
                    Layout.fillWidth: true
                    color: "#ffffff"
                    font.pointSize: 12
                    text: qsTr("Skip files that already exist")
                    verticalAlignment: Text.AlignVCenter
                }

                WToggle {
                    Layout.preferredWidth: 46
                    Layout.preferredHeight: 24
                    checked: app.settings.skipDownloaded
                    onToggled: app.settings.setSkipDownloaded(checked)
                }
            }

            // Logging
            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Text {
                    Layout.fillWidth: true
                    color: "#ffffff"
                    font.pointSize: 12
                    text: qsTr("Enable logging")
                    verticalAlignment: Text.AlignVCenter
                }

                Text {
                    color: "#99ffffff"
                    font.pointSize: 11
                    text: qsTr("Open log folder")
                    verticalAlignment: Text.AlignVCenter

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.openLogFolder()
                    }
                }

                WToggle {
                    Layout.preferredWidth: 46
                    Layout.preferredHeight: 24
                    checked: app.settings.enableLogging
                    onToggled: app.settings.setEnableLogging(checked)
                }
            }

            Item {
                Layout.fillHeight: true
            }
        }
    }
}
