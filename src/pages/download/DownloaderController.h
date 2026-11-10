#ifndef DOWNLOADERCONTROLLER_H
#define DOWNLOADERCONTROLLER_H
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QRegularExpression>
#include <QHash>
#include <QVector>
#include "downloader/DownloadManager.h"
#include "DownloaderModel.h"
#include "pages/settings/Settings.h"

class DownloaderController : public QObject {
    Q_OBJECT
    Q_PROPERTY(DownloaderModel* downloaderVM READ downloaderVM CONSTANT)

public:
    explicit DownloaderController(QObject *parent = nullptr, Settings *settings = nullptr);

    ~DownloaderController() override;

    void addAlbumToDownload(QSharedPointer<Album> album, DownloadQuality quality);

    void fetchFullAlbumData(QSharedPointer<Album> album);

    void parseDownloadLinks(QSharedPointer<Album> album); // Legacy method

public:
    DownloaderModel *downloaderVM() { return m_Model; }

    // Parses a newline separated list of album/playlist links and queues them.
    // Returns the number of albums that were actually added, so the UI can
    // report success instead of clearing the user's input blindly.
    Q_INVOKABLE int importUrls(const QString &list);

public slots:
    void startDownload();

    void onDownloadListAdd(QSharedPointer<Album> album, DownloadQuality quality);

    void updateModelData();

    void addToDownloadList(const QString &List);

    void onThreadsNumberChanged();

    void onMaxConcurrentOperationsChanged();

    void requestAddAlbums(QVector<QSharedPointer<Album> > albums, DownloadQuality quality);

private:
    int addToDownloadListImpl(const QString &list);

    void onAlbumDataFetched(QSharedPointer<Album> album, DownloadQuality quality);

    void processSongDownloads(QSharedPointer<Album> album, DownloadQuality quality);

    void processArtDownloads(QSharedPointer<Album> album);

    void cancelAlbum(Album *album);

    void retryAlbum(Album *album);

    void fetchSongDownloadLinks(QSharedPointer<Song> song, DownloadQuality quality, QSharedPointer<Album> album);

    void onSongLinksFound(QSharedPointer<Song> song, DownloadQuality quality, QSharedPointer<Album> album);

    void downloadSongFile(QSharedPointer<Song> song, DownloadQuality quality, QSharedPointer<Album> album);

    DownloadQuality normalizeQuality(DownloadQuality quality) const;

signals:
    void OnAlbumListUpdated();

    void songDownloadFinished(QSharedPointer<Song> song);

    void albumDownloadFinished(QSharedPointer<Album> album);

    void logMessage(const QString &message);

private:
    DownloaderModel *m_Model = nullptr;
    AsyncDownloadManager *m_downloadManager = nullptr;
    Settings *m_settings = nullptr;
    QMap<QSharedPointer<Album>, DownloadQuality> m_addedAlbums;
    QHash<Album *, WDownloadReplyMemory *> m_albumFetchReplies;
    QHash<Album *, QVector<WDownloadReplyFile *> > m_artReplies;
};


#endif //DOWNLOADERCONTROLLER_H
