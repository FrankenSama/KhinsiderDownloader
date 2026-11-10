#ifndef DOWNLOADERCONTROLLER_H
#define DOWNLOADERCONTROLLER_H
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QObject>
#include <QRegularExpression>
#include <QSet>
#include <QHash>
#include <QVector>
#include <functional>
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

    // Fetches a khinsider.com page through Qt's network stack. libcurl cannot
    // be used for these: the site's Cloudflare setup rejects curl's TLS
    // fingerprint with a 403 managed challenge no matter which headers are
    // sent, while Qt's native stack gets through.
    void fetchPage(const QUrl &url,
                   std::function<void(QNetworkReply::NetworkError, const QByteArray &)> onFinished,
                   int attemptsLeft = 2);

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
    QNetworkAccessManager *m_http = nullptr;
    Settings *m_settings = nullptr;
    QMap<QSharedPointer<Album>, DownloadQuality> m_addedAlbums;
    QSet<Album *> m_albumsBeingFetched;
    QHash<Album *, QVector<WDownloadReplyFile *> > m_artReplies;
};


#endif //DOWNLOADERCONTROLLER_H
