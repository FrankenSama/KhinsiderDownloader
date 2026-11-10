import QtQuick 2.15
import QtQuick.Controls.Basic
import "../shared"

Rectangle
{
       color: Theme.surface
       id:root
       height: 800
       width: 600

       ListView
       {
              id: downloadList

              width: parent.width
              anchors.top: parent.top
              anchors.topMargin: 10
              anchors.bottom: queueFooter.top
              clip: true
              spacing: 5
              reuseItems: true
              model: app.downloaderController.downloaderVM
              ScrollBar.vertical: WScrollBar {
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
              color: Theme.textMuted
              font.pointSize: 12
              text: qsTr("No downloads yet.\nSearch for an album or paste links to get started.")
       }
       // Queue summary: overall progress, transferred size and speed. The
       // totals are invokable getters, so they are mirrored into properties
       // here and refreshed whenever the model reports a change.
       Item
       {
              id: queueFooter
              width: parent.width
              height: parent.height * 0.12
              anchors.bottom: parent.bottom
              visible: queueFooter.totalSongs > 0

              property int totalSongs: 0
              property int downloadedSongs: 0
              property real totalBytes: 0
              property real downloadedBytes: 0
              property real progress: 0
              property string speedLabel: ""

              function formatBytes(bytes) {
                     if (bytes >= 1024 * 1024 * 1024)
                            return (bytes / (1024 * 1024 * 1024)).toFixed(2) + " GB";
                     if (bytes >= 1024 * 1024)
                            return (bytes / (1024 * 1024)).toFixed(1) + " MB";
                     if (bytes >= 1024)
                            return (bytes / 1024).toFixed(0) + " KB";
                     return Math.round(bytes) + " B";
              }

              function formatSpeed(bytesPerSecond) {
                     if (bytesPerSecond >= 1024 * 1024)
                            return (bytesPerSecond / (1024 * 1024)).toFixed(2) + " MB/s";
                     if (bytesPerSecond >= 1024)
                            return (bytesPerSecond / 1024).toFixed(2) + " KB/s";
                     return bytesPerSecond + " B/s";
              }

              function refresh() {
                     var vm = app.downloaderController.downloaderVM;
                     queueFooter.totalSongs = vm.totalSongs();
                     queueFooter.downloadedSongs = vm.totalDownloadedSongs();
                     queueFooter.totalBytes = vm.totalSize();
                     queueFooter.downloadedBytes = vm.totalDownloadedSize();
                     // Advertised sizes are only an estimate. Once every file
                     // is done, report the full total instead of a number that
                     // looks like the queue is still a megabyte short.
                     if (queueFooter.totalBytes > 0 && queueFooter.downloadedSongs >= queueFooter.totalSongs) {
                            queueFooter.downloadedBytes = queueFooter.totalBytes;
                     }
                     queueFooter.progress = vm.totalProgress() / 100;
                     queueFooter.speedLabel = queueFooter.formatSpeed(vm.totalSpeed());
              }

              Component.onCompleted: queueFooter.refresh()

              Connections {
                     target: app.downloaderController.downloaderVM
                     function onTotalsChanged() {
                            queueFooter.refresh();
                     }
              }

              Rectangle {
                     width: parent.width
                     height: 1
                     color: Theme.borderSoft
              }

              // Thin overall progress bar across the top of the queueFooter.
              Rectangle {
                     width: parent.width
                     height: 3
                     color: Theme.borderSoft
                     visible: queueFooter.totalBytes > 0

                     Rectangle {
                            width: parent.width * queueFooter.progress
                            height: parent.height
                            color: Theme.accent

                            Behavior on width {
                                   NumberAnimation {
                                          duration: 200
                                          easing.type: Easing.OutQuad
                                   }
                            }
                     }
              }

              Row
              {
                     anchors.top: parent.top
                     anchors.topMargin: 8
                     width: parent.width
                     height: parent.height - 8

                     Text {
                            id: sizeText
                            width: parent.width / 2
                            height: parent.height
                            color: Theme.text
                            font.pointSize: 13
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                            text: queueFooter.totalBytes > 0
                                   ? queueFooter.formatBytes(queueFooter.downloadedBytes) + " / " + queueFooter.formatBytes(queueFooter.totalBytes)
                                   : queueFooter.downloadedSongs + " / " + queueFooter.totalSongs + " files"
                     }

                     Text {
                            id: speedText
                            width: parent.width / 2
                            height: parent.height
                            color: Theme.textMuted
                            font.pointSize: 13
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                            text: queueFooter.speedLabel
                            // Hide the speed once every queued file is done;
                            // a lingering "0 B/s" reads like a stalled queue.
                            visible: queueFooter.downloadedSongs < queueFooter.totalSongs
                     }
              }
       }


}
