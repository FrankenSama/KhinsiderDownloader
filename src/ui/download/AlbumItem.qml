import QtQuick 2.15
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
       function formatSpeed(bytesPerSecond) {
           if (bytesPerSecond >= 1024 * 1024) {
               return (bytesPerSecond / (1024 * 1024)).toFixed(2) + " MB/s";
           } else if (bytesPerSecond >= 1024) {
               return (bytesPerSecond / 1024).toFixed(2) + " KB/s";
           } else {
               return bytesPerSecond + " B/s";
           }
       }
       onDonwloadedSongsChanged:
       {
              fileslabel.text = "Files: " + donwloadedSongs + "/" + totalSongs;
       }

       onSpeedInBytesChanged:
       {
              downloadstatus.text =  "Speed: " + formatSpeed(speedInBytes);
       }

       states: [
              State {
                     name: "downloading"
                     PropertyChanges { target: percentagebar; color:"#4CAF50" }
                     PropertyChanges { target: mainrect; color:"#6C98C4" }
                     PropertyChanges { target: perclabel; visible: true}
                     PropertyChanges { target: downloadstatus; visible: true }
                     PropertyChanges { target: fileslabel; visible: true }
                     PropertyChanges { target: restartbutton; visible: false }
                     PropertyChanges { target: folderbutton; visible: false }

              },
              State {
                     name: "unparsed"
                     PropertyChanges { target: percentagebar; color:"transparent" }
                     PropertyChanges { target: mainrect; color:"#FFA000" }
                     PropertyChanges { target: perclabel; visible: false}
                     PropertyChanges { target: downloadstatus; visible: false }
                     PropertyChanges { target: fileslabel; visible: false }
                     PropertyChanges { target: restartbutton; visible: false }
                     PropertyChanges { target: folderbutton; visible: false }

              },
              State {
                     name: "parsed"
                     PropertyChanges { target: percentagebar; color:"transparent" }
                     PropertyChanges { target: mainrect; color:"#6C98C4" }
                     PropertyChanges { target: perclabel; visible: false}
                     PropertyChanges { target: downloadstatus; visible: true }
                     PropertyChanges { target: fileslabel; visible: true }
                      PropertyChanges { target: restartbutton; visible: false }
                     PropertyChanges { target: folderbutton; visible: false }

              },
              State {
                     name: "complete"
                     PropertyChanges { target: percentagebar; color:"#2E7D32" }
                     PropertyChanges { target: mainrect; color:"#4CAF50" }
                     PropertyChanges { target: perclabel; visible: false}
                     PropertyChanges { target: downloadstatus; visible: false }
                     PropertyChanges { target: fileslabel; visible: true }
                     PropertyChanges { target: restartbutton; visible: false }
                     PropertyChanges { target: folderbutton; visible: true }

              },
              State {
                     name: "errored"
                     PropertyChanges { target: percentagebar; color:"transparent" }
                     PropertyChanges { target: mainrect; color:"#D32F2F" }
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
              color: "#6C98C4" // background
              width: parent.width
              height: parent.height
              radius: 20

              // Progress fill, drawn behind the card content.
              Rectangle
              {
                     property int percentage;
                     id: percentagebar
                     color: "#4CAF50"
                     width: (percentagebar.percentage / 100) * mainrect.width
                     height: parent.height
                     radius: 20

                     onPercentageChanged:
                     {
                            percentagebar.width = (percentagebar.percentage / 100) * mainrect.width
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
                                   color:"white"
                                   verticalAlignment: Text.AlignVCenter
                                   font.bold: false
                                   font.pointSize: 13
                            }

                            Text
                            {
                                   id: perclabel;
                                   Layout.preferredWidth: 44
                                   text: percentagebar.percentage + "%"
                                   color:"white"
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
                                   color: "#e6ffffff"
                                   horizontalAlignment: Text.AlignLeft
                                   verticalAlignment: Text.AlignVCenter
                                   font.pointSize: 10
                            }

                            Text {
                                   id: downloadstatus
                                   Layout.preferredWidth: 130
                                   text: ""
                                   elide: Text.ElideRight
                                   color: "#e6ffffff"
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

                                   ColorOverlay {
                                          anchors.fill: parent
                                          source: parent
                                          color: "white"
                                   }
                                   MouseArea
                                   {
                                         width: parent.width
                                         height: parent.height
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
                                   MouseArea
                                   {
                                         width: parent.width
                                         height: parent.height
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
                                   MouseArea
                                   {
                                       width: parent.width
                                       height: parent.height
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
