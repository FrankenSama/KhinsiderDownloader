import QtQuick 2.15
import QtQuick.Controls.Basic
import Qt5Compat.GraphicalEffects
import QtQuick.Layouts

Item {
       state: "errored"

       signal cancelRequested();
       signal retryRequested();
       signal openFolderRequested();
       property var donwloadedSongs;
       property var totalSongs;
       property var speedInBytes;
       property var downloadedBytes;
       property var totalBytes;
       property var errorMessage;

       function formatSpeed(bytesPerSecond) {
           if (bytesPerSecond >= 1024 * 1024) {
               return (bytesPerSecond / (1024 * 1024)).toFixed(2) + " MB/s";
           } else if (bytesPerSecond >= 1024) {
               return (bytesPerSecond / 1024).toFixed(2) + " KB/s";
           } else {
               return Math.round(bytesPerSecond) + " B/s";
           }
       }

       function formatSizePair(downloaded, total) {
           if (total >= 1024 * 1024 * 1024)
               return (downloaded / (1024 * 1024 * 1024)).toFixed(2) + " / "
                      + (total / (1024 * 1024 * 1024)).toFixed(1) + " GB";
           if (total >= 1024 * 1024)
               return (downloaded / (1024 * 1024)).toFixed(1) + " / "
                      + (total / (1024 * 1024)).toFixed(1) + " MB";
           if (total >= 1024)
               return (downloaded / 1024).toFixed(0) + " / "
                      + (total / 1024).toFixed(0) + " KB";
           return Math.round(downloaded) + " / " + Math.round(total) + " B";
       }

       function formatEta() {
           if (!speedInBytes || speedInBytes <= 0 || !totalBytes || totalBytes <= 0)
               return "";
           var remaining = Math.max(0, totalBytes - (downloadedBytes || 0));
           if (remaining <= 0)
               return "";
           var seconds = Math.round(remaining / speedInBytes);
           if (seconds < 60)
               return seconds + "s left";
           var minutes = Math.floor(seconds / 60);
           var secs = seconds % 60;
           if (minutes < 60)
               return minutes + "m " + (secs < 10 ? "0" : "") + secs + "s left";
           var hours = Math.floor(minutes / 60);
           return hours + "h " + (minutes % 60) + "m left";
       }

       function updateStats() {
              if (state === "errored" && errorMessage && errorMessage.length > 0) {
                     fileslabel.text = errorMessage;
              } else {
                     fileslabel.text = "Files: " + donwloadedSongs + "/" + totalSongs;
              }

              if (totalBytes > 0) {
                     var downloaded = downloadedBytes || 0;
                     // Advertised sizes are estimates; once every file is on
                     // disk report the full total instead of a short number.
                     if (totalSongs > 0 && donwloadedSongs >= totalSongs)
                            downloaded = totalBytes;
                     sizelabel.text = formatSizePair(downloaded, totalBytes);
              } else {
                     sizelabel.text = "";
              }

              var status = "";
              if (speedInBytes > 0) {
                     status = formatSpeed(speedInBytes);
                     var eta = formatEta();
                     if (eta.length > 0)
                            status += "  ·  " + eta;
              }
              downloadstatus.text = status;
       }

       onDonwloadedSongsChanged: updateStats()
       onTotalSongsChanged: updateStats()
       onDownloadedBytesChanged: updateStats()
       onTotalBytesChanged: updateStats()
       onSpeedInBytesChanged: updateStats()
       onErrorMessageChanged: updateStats()
       onStateChanged: updateStats()

       states: [
              State {
                     name: "downloading"
                     PropertyChanges { target: statusRail; color: Theme.accent }
                     PropertyChanges { target: percentagebar; color: Theme.accent; visible: true }
                     PropertyChanges { target: perclabel; visible: true}
                     PropertyChanges { target: downloadstatus; visible: true }
                     PropertyChanges { target: fileslabel; visible: true }
                     PropertyChanges { target: restartbutton; visible: false }
                     PropertyChanges { target: folderbutton; visible: false }

              },
              State {
                     name: "unparsed"
                     PropertyChanges { target: statusRail; color: Theme.warning }
                     PropertyChanges { target: percentagebar; color: "transparent"; visible: false }
                     PropertyChanges { target: perclabel; visible: false}
                     PropertyChanges { target: downloadstatus; visible: false }
                     PropertyChanges { target: fileslabel; visible: false }
                     PropertyChanges { target: restartbutton; visible: false }
                     PropertyChanges { target: folderbutton; visible: false }

              },
              State {
                     name: "parsed"
                     PropertyChanges { target: statusRail; color: Theme.textFaint }
                     PropertyChanges { target: percentagebar; color: "transparent"; visible: false }
                     PropertyChanges { target: perclabel; visible: false}
                     PropertyChanges { target: downloadstatus; visible: true }
                     PropertyChanges { target: fileslabel; visible: true }
                      PropertyChanges { target: restartbutton; visible: false }
                     PropertyChanges { target: folderbutton; visible: false }

              },
              State {
                     name: "complete"
                     PropertyChanges { target: statusRail; color: Theme.success }
                     PropertyChanges { target: percentagebar; color: "transparent"; visible: false }
                     PropertyChanges { target: perclabel; visible: false}
                     PropertyChanges { target: downloadstatus; visible: false }
                     PropertyChanges { target: fileslabel; visible: true }
                     PropertyChanges { target: restartbutton; visible: false }
                     PropertyChanges { target: folderbutton; visible: true }

              },
              State {
                     name: "errored"
                     PropertyChanges { target: statusRail; color: Theme.danger }
                     PropertyChanges { target: percentagebar; color: "transparent"; visible: false }
                     PropertyChanges { target: perclabel; visible: false}
                     PropertyChanges { target: downloadstatus; visible: false }
                     PropertyChanges { target: fileslabel; visible: true }
                     PropertyChanges { target: restartbutton; visible: true }
                     PropertyChanges { target: folderbutton; visible: false }

              }
       ]
       property alias progress: percentagebar.percentage;
       property alias label: albumname.text
       height: 56
       width: 400
       Rectangle
       {
              id:mainrect
              color: Theme.surfaceAlt // background
              border.width: 1
              border.color: Theme.border
              width: parent.width
              height: parent.height
              radius: Theme.radiusMd

              // Status rail: accent while downloading, green when complete,
              // red on error, grey when idle.
              Rectangle
              {
                     id: statusRail
                     color: Theme.textFaint
                     width: 3
                     height: parent.height - 16
                     radius: 2
                     anchors.left: parent.left
                     anchors.leftMargin: 6
                     anchors.verticalCenter: parent.verticalCenter

                     Behavior on color {
                         ColorAnimation {
                             duration: 150
                         }
                     }
              }

              // Progress fill along the bottom edge of the card.
              Rectangle
              {
                     property int percentage;
                     id: percentagebar
                     color: Theme.accent
                     anchors.left: parent.left
                     anchors.bottom: parent.bottom
                     anchors.leftMargin: 1
                     anchors.bottomMargin: 1
                     width: (percentagebar.percentage / 100) * (mainrect.width - 2)
                     height: 3
                     radius: 1.5

                     onPercentageChanged:
                     {
                            percentagebar.width = (percentagebar.percentage / 100) * (mainrect.width - 2)
                     }

                     Behavior on width {
                         NumberAnimation {
                             duration: 100
                             easing.type: Easing.InOutQuad
                         }
                     }
              }

              // Everything below is always visible. Previously the album name and
              // percentage were hidden on hover, and the actions only existed
              // inside that hover overlay, so they were undiscoverable.
              ColumnLayout {
                     anchors.fill: parent
                     anchors.leftMargin: 12
                     anchors.rightMargin: 10
                     anchors.topMargin: 6
                     anchors.bottomMargin: 6
                     spacing: 0

                     RowLayout {
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            Text
                            {
                                   id: albumname;
                                   Layout.fillWidth: true
                                   text: ""
                                   elide: Text.ElideRight
                                   horizontalAlignment: Text.AlignLeft
                                   color: Theme.text
                                   verticalAlignment: Text.AlignVCenter
                                   font.bold: false
                                   font.pointSize: 13
                            }

                            Text
                            {
                                   id: sizelabel;
                                   Layout.preferredWidth: 94
                                   visible: text.length > 0
                                   text: ""
                                   color: Theme.textMuted
                                   elide: Text.ElideRight
                                   horizontalAlignment: Text.AlignRight
                                   verticalAlignment: Text.AlignVCenter
                                   font.pointSize: 9
                            }

                            Text
                            {
                                   id: perclabel;
                                   Layout.preferredWidth: 44
                                   text: percentagebar.percentage + "%"
                                   color: Theme.text
                                   elide: Text.ElideRight
                                   horizontalAlignment: Text.AlignRight
                                   verticalAlignment: Text.AlignVCenter
                                   font.bold: false
                                   font.pointSize: 13
                            }
                     }

                     RowLayout {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            spacing: 8

                            Text {
                                   id: fileslabel
                                   Layout.fillWidth: true
                                   text: "Files: 0/0"
                                   elide: Text.ElideRight
                                   color: state === "errored" ? Theme.danger : Theme.textMuted
                                   horizontalAlignment: Text.AlignLeft
                                   verticalAlignment: Text.AlignVCenter
                                   font.pointSize: 10
                            }

                            Text {
                                   id: downloadstatus
                                   Layout.preferredWidth: 130
                                   text: ""
                                   elide: Text.ElideRight
                                   color: Theme.textMuted
                                   horizontalAlignment: Text.AlignRight
                                   verticalAlignment: Text.AlignVCenter
                                   font.pointSize: 10
                            }

                            Image {
                                   id: restartbutton
                                   Layout.preferredWidth: 18
                                   Layout.preferredHeight: 18
                                   Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                   visible: false
                                   property string iconFallback: "qrc:/icons/retry.svg"
                                   source: "../../../icons/retry.svg"
                                   fillMode: Image.PreserveAspectFit

                                   ToolTip.visible: retryArea.containsMouse
                                   ToolTip.delay: 400
                                   ToolTip.text: qsTr("Retry download")

                                   ColorOverlay {
                                          anchors.fill: parent
                                          source: parent
                                          color: Theme.text
                                   }
                                   MouseArea
                                   {
                                         width: parent.width
                                         height: parent.height
                                         id: retryArea
                                         hoverEnabled: true
                                         cursorShape: Qt.PointingHandCursor
                                         onClicked:
                                         {
                                             retryRequested();
                                         }
                                   }

                                   onStatusChanged: {
                                       if (status === Image.Error) {
                                           source = iconFallback;
                                       }
                                   }
                            }

                            Image {
                                   id: deletebutton
                                   Layout.preferredWidth: 18
                                   Layout.preferredHeight: 18
                                   Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                   property string iconFallback: "qrc:/icons/delete.svg"
                                   source: "../../../icons/delete.svg"
                                   fillMode: Image.PreserveAspectFit

                                   ToolTip.visible: removeArea.containsMouse
                                   ToolTip.delay: 400
                                   ToolTip.text: qsTr("Remove from queue")
                                   MouseArea
                                   {
                                         width: parent.width
                                         height: parent.height
                                         id: removeArea
                                         hoverEnabled: true
                                         cursorShape: Qt.PointingHandCursor
                                         onClicked:
                                         {
                                                cancelRequested();
                                         }
                                   }

                                   onStatusChanged: {
                                       if (status === Image.Error) {
                                           source = iconFallback;
                                       }
                                   }
                            }
                            Image {
                                   id: folderbutton
                                   Layout.preferredWidth: 18
                                   Layout.preferredHeight: 18
                                   Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                   visible: false
                                   property string iconFallback: "qrc:/icons/folder.svg"
                                   source: "../../../icons/folder.svg"
                                   fillMode: Image.PreserveAspectFit

                                   ToolTip.visible: folderArea.containsMouse
                                   ToolTip.delay: 400
                                   ToolTip.text: qsTr("Open album folder")

                                   MouseArea
                                   {
                                       id: folderArea
                                       width: parent.width
                                       height: parent.height
                                       hoverEnabled: true
                                       cursorShape: Qt.PointingHandCursor
                                       onClicked:
                                       {
                                           openFolderRequested();
                                       }
                                   }

                                   onStatusChanged: {
                                       if (status === Image.Error) {
                                           source = iconFallback;
                                       }
                                   }
                            }

                     }
              }
       }
}
