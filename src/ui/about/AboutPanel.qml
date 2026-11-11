import QtQuick
import "../shared"
import QtQml
import QtQuick.Controls.Basic
import QtQml.XmlListModel
import QtQuick.Layouts

Rectangle {
    id: root
    color: Theme.bg
    height: 700
    width: 400

    readonly property string repoUrl: "https://github.com/FrankenSama/KhinsiderDownloader"
    readonly property string releasesUrl: root.repoUrl + "/releases"
    readonly property string issuesUrl: root.repoUrl + "/issues"
    readonly property string siteUrl: "https://downloads.khinsider.com"

    // APP_VERSION carries a build-channel suffix ("3.1.0.44-release"); the pill
    // only shows the numeric version.
    readonly property string versionLabel: {
        var version = app.aboutController.appVersion;
        if (version.toLowerCase().endsWith("-release"))
            version = version.slice(0, -"-release".length);
        return "v" + version;
    }

    // The startup check only opens the update dialog. The inline status line is
    // reserved for checks the user started from this page.
    property bool manualCheck: false
    property string updateStatus: ""
    property color updateStatusColor: Theme.textMuted

    function checkForUpdates() {
        root.manualCheck = true;
        root.updateStatus = qsTr("Checking for updates…");
        root.updateStatusColor = Theme.textMuted;
        app.aboutController.checkForUpdates();
    }

    function openUrl(url) {
        Qt.openUrlExternally(url);
    }

    Connections {
        target: app.aboutController

        function onFoundNewUpdate(version) {
            messageDialog.visible = true;
            if (root.manualCheck) {
                root.updateStatus = qsTr("Version %1 is available").arg(version);
                root.updateStatusColor = Theme.accent;
            }
            root.manualCheck = false;
        }

        function onUpToDate() {
            if (root.manualCheck) {
                root.updateStatus = qsTr("You're running the latest version");
                root.updateStatusColor = Theme.success;
            }
            root.manualCheck = false;
        }

        function onUpdateCheckFailed() {
            if (root.manualCheck) {
                root.updateStatus = qsTr("Couldn't reach GitHub - check your connection");
                root.updateStatusColor = Theme.danger;
            }
            root.manualCheck = false;
        }
    }

    UpdateCheckerDialog {
        id: messageDialog
        onAccepted: root.openUrl(root.releasesUrl)
        Component.onCompleted: visible = false
    }

    Flickable {
        id: scroll
        anchors.fill: parent
        clip: true
        contentWidth: width
        contentHeight: content.implicitHeight + 32
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: WScrollBar { }

        ColumnLayout {
            id: content
            x: (scroll.width - width) / 2
            y: 16
            width: Math.min(scroll.width - 32, 760)
            spacing: 14

            // App identity
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: headerRow.implicitHeight + 32
                radius: Theme.radiusLg
                color: Theme.surface
                border.width: 1
                border.color: Theme.borderSoft

                RowLayout {
                    id: headerRow
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 16

                    Rectangle {
                        Layout.preferredWidth: 72
                        Layout.preferredHeight: 72
                        radius: Theme.radiusLg
                        color: "#ffffff"
                        clip: true

                        Image {
                            anchors.fill: parent
                            anchors.margins: 4
                            source: "qrc:/icons/icon.png"
                            fillMode: Image.PreserveAspectFit
                            smooth: true
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 6

                        Text {
                            color: Theme.text
                            font.pointSize: 22
                            font.bold: true
                            text: qsTr("Khinsider Downloader")
                        }

                        RowLayout {
                            spacing: 8

                            Rectangle {
                                radius: Theme.radiusSm
                                color: Theme.accentSoft
                                implicitWidth: versionText.implicitWidth + 16
                                implicitHeight: 22

                                Text {
                                    id: versionText
                                    anchors.centerIn: parent
                                    color: Theme.accent
                                    font.pointSize: 10
                                    font.bold: true
                                    text: root.versionLabel
                                }
                            }

                            Rectangle {
                                radius: Theme.radiusSm
                                color: Theme.surfaceAlt
                                border.width: 1
                                border.color: Theme.borderSoft
                                implicitWidth: licenseText.implicitWidth + 16
                                implicitHeight: 22

                                Text {
                                    id: licenseText
                                    anchors.centerIn: parent
                                    color: Theme.textMuted
                                    font.pointSize: 10
                                    text: qsTr("GPLv3")
                                }
                            }
                        }

                        Text {
                            Layout.fillWidth: true
                            Layout.topMargin: 2
                            color: Theme.textMuted
                            font.pointSize: 12
                            wrapMode: Text.WordWrap
                            text: qsTr("Browse and download video game soundtracks from KHInsider, "
                                       + "with album art, quality selection and multi-threaded downloads.")
                        }
                    }
                }
            }

            // Primary actions
            Flow {
                Layout.fillWidth: true
                spacing: 8

                WButton {
                    width: 172
                    height: 40
                    fontSize: 13
                    secondary: true
                    label: qsTr("Check for updates")
                    onClicked: root.checkForUpdates()
                }

                WButton {
                    width: 120
                    height: 40
                    fontSize: 13
                    secondary: true
                    label: qsTr("GitHub")
                    onClicked: root.openUrl(root.repoUrl)
                }

                WButton {
                    width: 150
                    height: 40
                    fontSize: 13
                    secondary: true
                    label: qsTr("Report an issue")
                    onClicked: root.openUrl(root.issuesUrl)
                }

                WButton {
                    width: 130
                    height: 40
                    fontSize: 13
                    secondary: true
                    label: qsTr("KHInsider")
                    onClicked: root.openUrl(root.siteUrl)
                }
            }

            Text {
                Layout.fillWidth: true
                Layout.preferredHeight: visible ? implicitHeight : 0
                visible: root.updateStatus.length > 0
                color: root.updateStatusColor
                font.pointSize: 11
                wrapMode: Text.WordWrap
                text: root.updateStatus
            }

            // Links
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: linksColumn.implicitHeight + 24
                radius: Theme.radiusLg
                color: Theme.surface
                border.width: 1
                border.color: Theme.borderSoft

                ColumnLayout {
                    id: linksColumn
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 2

                    Text {
                        Layout.leftMargin: 8
                        Layout.bottomMargin: 4
                        color: Theme.textMuted
                        font.pointSize: 10
                        font.bold: true
                        text: qsTr("LINKS")
                    }

                    Repeater {
                        model: [
                            {
                                title: qsTr("Repository"),
                                detail: qsTr("Source code on GitHub"),
                                url: root.repoUrl
                            },
                            {
                                title: qsTr("Releases"),
                                detail: qsTr("Download the latest version"),
                                url: root.releasesUrl
                            },
                            {
                                title: qsTr("Issue tracker"),
                                detail: qsTr("Report bugs and request features"),
                                url: root.issuesUrl
                            },
                            {
                                title: qsTr("KHInsider"),
                                detail: qsTr("downloads.khinsider.com"),
                                url: root.siteUrl
                            }
                        ]

                        delegate: Rectangle {
                            Layout.fillWidth: true
                            implicitHeight: 34
                            radius: Theme.radiusSm
                            color: linkMouse.containsMouse ? Theme.surfaceHover : "transparent"

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 8
                                anchors.rightMargin: 8
                                spacing: 10

                                Text {
                                    color: Theme.text
                                    font.pointSize: 12
                                    text: modelData.title
                                }

                                Item {
                                    Layout.fillWidth: true
                                }

                                Text {
                                    color: Theme.accent
                                    font.pointSize: 11
                                    text: modelData.detail
                                    elide: Text.ElideRight
                                }
                            }

                            MouseArea {
                                id: linkMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: root.openUrl(modelData.url)
                            }
                        }
                    }
                }
            }

            // Built with
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: builtWithColumn.implicitHeight + 24
                radius: Theme.radiusLg
                color: Theme.surface
                border.width: 1
                border.color: Theme.borderSoft

                ColumnLayout {
                    id: builtWithColumn
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 10

                    Text {
                        Layout.leftMargin: 8
                        color: Theme.textMuted
                        font.pointSize: 10
                        font.bold: true
                        text: qsTr("BUILT WITH")
                    }

                    Flow {
                        Layout.fillWidth: true
                        Layout.leftMargin: 8
                        Layout.rightMargin: 8
                        spacing: 8

                        Repeater {
                            model: ["Qt 6", "libxml2", "libcurl", "CMake", "vcpkg"]

                            delegate: Rectangle {
                                radius: Theme.radiusSm
                                color: Theme.surfaceAlt
                                border.width: 1
                                border.color: Theme.borderSoft
                                implicitWidth: chipText.implicitWidth + 18
                                implicitHeight: 24

                                Text {
                                    id: chipText
                                    anchors.centerIn: parent
                                    color: Theme.textMuted
                                    font.pointSize: 10
                                    text: modelData
                                }
                            }
                        }
                    }
                }
            }

            // Contributors
            Text {
                Layout.topMargin: 4
                color: Theme.textMuted
                font.pointSize: 11
                font.bold: true
                text: qsTr("THANKS TO")
            }

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: thanksColumn.implicitHeight + 16
                radius: Theme.radiusLg
                color: Theme.surface
                border.width: 1
                border.color: Theme.borderSoft

                ColumnLayout {
                    id: thanksColumn
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 0

                    XmlListModel {
                        id: contributorsModel
                        source: "qrc:/CONTRIBUTORS.xml"
                        query: "/contributors/contributor"

                        XmlListModelRole {
                            name: "username"; elementName: "username"
                        }
                        XmlListModelRole {
                            name: "contributionType"; elementName: "contributionType"
                        }
                    }

                    Repeater {
                        model: contributorsModel

                        delegate: Item {
                            Layout.fillWidth: true
                            implicitHeight: 30

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 8
                                anchors.rightMargin: 8
                                spacing: 10

                                Text {
                                    color: Theme.text
                                    font.pointSize: 12
                                    text: username
                                }

                                Item {
                                    Layout.fillWidth: true
                                }

                                Text {
                                    color: Theme.textMuted
                                    font.pointSize: 11
                                    text: contributionType
                                    elide: Text.ElideRight
                                }
                            }
                        }
                    }
                }
            }

            // Colophon
            Text {
                Layout.fillWidth: true
                Layout.topMargin: 4
                Layout.bottomMargin: 8
                horizontalAlignment: Text.AlignHCenter
                color: Theme.textFaint
                font.pointSize: 10
                wrapMode: Text.WordWrap
                textFormat: Text.RichText
                linkColor: Theme.accent
                text: qsTr("© 2025 Weespin · Free software under the "
                           + "<a href=\"https://www.gnu.org/licenses/gpl-3.0.html\">GPLv3</a>")
                onLinkActivated: root.openUrl(link)
            }
        }
    }
}
