import QtQuick 2.15
import QtQuick.Controls.Basic

Rectangle
{
       color: "#34495E"
       id:root
       height: 800
       width: 600

       ListView
       {
              id: downloadList

              width: parent.width
              anchors.top: parent.top
              anchors.topMargin: 10
              anchors.bottom: s.top
              clip: true
              spacing: 5
              reuseItems: true
              model: app.downloaderController.downloaderVM
              ScrollBar.vertical: ScrollBar {
                     policy: ScrollBar.AlwaysOn
              }

              delegate: Item {
                     width: downloadList.width
                     height: 56

                     AlbumItem
                     {
                            anchors.horizontalCenter: parent.horizontalCenter
                            width: parent.width * 0.92
                            height: parent.height
                            progress: model.progress
                            state: model.state
                            label: model.name
                            donwloadedSongs: model.downloadedSongs
                            totalSongs: model.totalSongs
                            speedInBytes: model.speed;
                            onCancelRequested:
                            {
                                   app.downloaderController.downloaderVM.cancelAlbum(index)
                            }
                            onRetryRequested :
                            {
                                   app.downloaderController.downloaderVM.retryAlbum(index);
                            }
                            onOpenFolderRequested:
                            {
                                   app.downloaderController.downloaderVM.openAlbumFolder(index);
                            }
                     }
              }
       }

       Text
       {
              anchors.horizontalCenter: parent.horizontalCenter
              anchors.verticalCenter: downloadList.verticalCenter
              width: parent.width * 0.8
              visible: downloadList.count === 0
              horizontalAlignment: Text.AlignHCenter
              wrapMode: Text.WordWrap
              color: "#b5ffffff"
              font.pointSize: 12
              text: qsTr("No downloads yet.\nSearch for an album or paste links to get started.")
       }
       Item
       {
              id: s
              width: parent.width
              height: parent.height * 0.1
              anchors.bottom: parent.bottom
              Rectangle {
                     width: parent.width
                     height: 1
                     color: "#33ffffff"
                     // The footer is empty until something is queued; don't
                     // leave a divider floating in an empty panel.
                     visible: speedText.opacity > 0
              }
              Row
              {
                     width: parent.width
                     height: parent.height
                     Text {
                            id: speedText
                            height: parent.height
                            width: parent.width/2
                            text: "";
                            color: "#ffffff"
                            opacity: 0
                            font.pointSize: 16
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            function formatSpeed(bytesPerSecond) {
                                   if (bytesPerSecond >= 1024 * 1024) {
                                          return (bytesPerSecond / (1024 * 1024)).toFixed(2) + " MB/s";
                                   } else if (bytesPerSecond >= 1024) {
                                          return (bytesPerSecond / 1024).toFixed(2) + " KB/s";
                                   } else {
                                          return bytesPerSecond + " B/s";
                                   }
                            }
                            Behavior on opacity {
                                 NumberAnimation {
                                     duration: 100
                                     easing.type: Easing.InOutQuad
                                 }
                             }
                            Connections {
                                   target: app.downloaderController.downloaderVM
                                   function onTotalsChanged()
                                   {
                                          var shouldShowAnyData = app.downloaderController.downloaderVM.totalSongs() !== 0 ? 1 : 0;
                                          speedText.opacity = shouldShowAnyData ? 1 : 0;
                                          if(shouldShowAnyData)
                                          {
                                                 speedText.text = speedText.formatSpeed(app.downloaderController.downloaderVM.totalSpeed());
                                          }
                                   }
                            }
                     }


                     Text {
                            id: downloadedSizeText
                            height: parent.height
                            width: parent.width/2
                            font.pointSize: 16
                            color: "#ffffff"
                            opacity: 0
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            Behavior on opacity {
                                 NumberAnimation {
                                     duration: 100
                                     easing.type: Easing.InOutQuad
                                 }
                             }
                            Connections {
                                   target: app.downloaderController.downloaderVM
                                   function onTotalsChanged()
                                   {
                                          var shouldShowAnyData = app.downloaderController.downloaderVM.totalSongs() !== 0 ? 1 : 0;
                                          downloadedSizeText.opacity = shouldShowAnyData ? 1 : 0;
                                          if(shouldShowAnyData)
                                          {
                                                 downloadedSizeText.text = app.downloaderController.downloaderVM.totalDownloadedSongs()
                                                               + "/" + app.downloaderController.downloaderVM.totalSongs();
                                          }
                                   }
                            }
                     }
              }


       }
}
