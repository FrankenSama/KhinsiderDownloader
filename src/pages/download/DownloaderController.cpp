#include "DownloaderController.h"
#include "Album.h"

#include <QDir>
#include <QFileInfo>
#include <QTimer>
#include <QUrl>

#include "KhinsiderRequest.h"

static const QMap<QString, int> qualityRanking = {
    {"FLAC", 4},
    {"ALAC", 3},
    {"AAC", 2},
    {"MP3", 1}
};

DownloaderController::DownloaderController(QObject *parent, Settings *settings)
    : QObject(parent), m_settings(settings) {
    // The manager must outlive every reply owned by this controller. Making it
    // a child of the controller guarantees that its cleanup runs first, and
    // replies parented to the manager are released after the workers stopped.
    m_downloadManager = new AsyncDownloadManager(this);
    m_http = new QNetworkAccessManager(this);
    m_Model = new DownloaderModel(this);
    connect(m_Model, &DownloaderModel::addToDownloadList, this, &DownloaderController::addToDownloadList);
    connect(m_Model, &DownloaderModel::albumCancelRequested, this, &DownloaderController::cancelAlbum);
    connect(m_Model, &DownloaderModel::albumRetryRequested, this, &DownloaderController::retryAlbum);

    if (m_settings) {
        m_downloadManager->setDownloadLimitPerThread(m_settings->maxConcurrentDownloadsPerThread());
        m_downloadManager->setThreadCount(m_settings->downloadThreads());
    }
}

DownloaderController::~DownloaderController() {
    // Stop all queued reply callbacks from entering the controller while its
    // members are being torn down.
    QObject::disconnect(nullptr, nullptr, this, nullptr);

    // Destroy the manager (and its reply children) while all controller
    // members are still alive. Its shutdown loop may deliver queued
    // downloadFinished callbacks, which must not run after the hashes below
    // have been destroyed.
    delete m_downloadManager;
    m_downloadManager = nullptr;
}

DownloadQuality DownloaderController::normalizeQuality(DownloadQuality quality) const {
    if (quality != DownloadQuality::UNKNOWN) {
        return quality;
    }

    if (m_settings) {
        const DownloadQuality preferred = m_settings->preferredAudioQuality();
        if (preferred == DownloadQuality::MP3 || preferred == DownloadQuality::BEST) {
            return preferred;
        }
    }

    return DownloadQuality::MP3;
}

void DownloaderController::addAlbumToDownload(QSharedPointer<Album> album, DownloadQuality quality) {
    if (!album) {
        return;
    }
    m_addedAlbums[album] = normalizeQuality(quality);
}

void DownloaderController::fetchPage(
    const QUrl &url,
    std::function<void(QNetworkReply::NetworkError, const QByteArray &)> onFinished,
    int attemptsLeft) {
    QNetworkReply *reply = m_http->get(khinsiderRequest(url));

    connect(reply, &QNetworkReply::finished, this,
            [this, reply, url, onFinished, attemptsLeft]() {
                const QNetworkReply::NetworkError error = reply->error();
                const int status =
                    reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                const QByteArray body = reply->readAll();
                reply->deleteLater();

                if (error == QNetworkReply::NoError) {
                    onFinished(error, body);
                    return;
                }

                // Retry once on a transient failure. A 403 is the Cloudflare
                // challenge and a 404 will not get better either, so both are
                // reported immediately.
                const bool worthRetrying =
                    error != QNetworkReply::OperationCanceledError && status != 403 &&
                    status != 404;
                if (worthRetrying && attemptsLeft > 1) {
                    qWarning() << "Retrying" << url.toString() << "after" << reply->errorString();
                    QTimer::singleShot(600, this, [this, url, onFinished, attemptsLeft]() {
                        fetchPage(url, onFinished, attemptsLeft - 1);
                    });
                    return;
                }

                qWarning() << "Request failed:" << url.toString() << reply->errorString()
                           << "status" << status;
                onFinished(error, body);
            });
}

void DownloaderController::fetchFullAlbumData(QSharedPointer<Album> album) {
    if (!album || album->isInfoParsed()) {
        return;
    }

    // Avoid starting a second request for an album whose metadata is already
    // being fetched (for example when the user clicks retry twice).
    if (m_albumsBeingFetched.contains(album.data())) {
        return;
    }
    m_albumsBeingFetched.insert(album.data());

    fetchPage(QUrl(album->albumLink()),
              [this, album](QNetworkReply::NetworkError error, const QByteArray &body) {
                  m_albumsBeingFetched.remove(album.data());

                  // The album was canceled while the metadata request was in
                  // flight. Do not resurrect it in m_addedAlbums.
                  auto albumIt = m_addedAlbums.constFind(album);
                  if (albumIt == m_addedAlbums.constEnd()) {
                      return;
                  }

                  if (error == QNetworkReply::OperationCanceledError) {
                      return;
                  }
                  if (error != QNetworkReply::NoError) {
                      album->setHasErrors(true);
                      return;
                  }

                  htmlDocPtr doc = htmlReadMemory(body.constData(), body.size(), nullptr,
                                                  nullptr,
                                                  HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
                  if (!doc) {
                      qWarning() << "HTML read failed";
                      album->setHasErrors(true);
                      return;
                  }

                  bool parsed = false;
                  if (album->isPlaylist()) {
                      parsed = KhinsiderParser::ParsePlaylist(doc, album);
                  } else {
                      parsed = KhinsiderParser::ParseAlbumFullData(doc, album);
                  }
                  xmlFreeDoc(doc);

                  if (!parsed) {
                      album->setHasErrors(true);
                      return;
                  }

                  onAlbumDataFetched(album, albumIt.value());
              });
}


void DownloaderController::onAlbumDataFetched(QSharedPointer<Album> album, DownloadQuality quality) {
    processSongDownloads(album, quality);
}

void DownloaderController::processSongDownloads(QSharedPointer<Album> album, DownloadQuality quality)
{
    if (!album) {
        return;
    }

    quality = normalizeQuality(quality);
    album->setIsDownloading(true);

    if (m_settings && m_settings->downloadArt()) {
        processArtDownloads(album);
    }

    for (const auto &song: album->songs())
    {
        if (!song) {
            continue;
        }

        if (!song->getAllDownloadLinksParsed()) {
            fetchSongDownloadLinks(song, quality, album);
        } else {
            downloadSongFile(song, quality, album);
        }
    }
}

void DownloaderController::processArtDownloads(QSharedPointer<Album> album) {
    if (!album) {
        return;
    }

    const QString downloadPath = m_settings ? m_settings->downloadPath() : QString();
    if (downloadPath.isEmpty()) {
        return;
    }

    const QString invalidChars = "\\/:*?\"<>|";
    QString albumName = album->name();
    if (albumName.isEmpty()) {
        albumName = "Unknown Album";
    }
    for (QChar c: invalidChars) {
        albumName.remove(c);
    }

    QDir albumDir(downloadPath + QDir::separator() + albumName + QDir::separator() + "Art");
    if (!albumDir.exists() && !albumDir.mkpath(".")) {
        qWarning() << "Failed to create art directory" << albumDir.path();
        album->setHasErrors(true);
        return;
    }

    for (const auto &downloadLink: album->albumImage())
    {
        QUrl url(downloadLink);
        QString fileName = QUrl::fromPercentEncoding(url.path().section('/', -1).toUtf8());

        for (const QChar &c: invalidChars) {
            fileName.replace(c, '_');
        }
        if (fileName.isEmpty()) {
            fileName = "art";
        }

        QString filePath = albumDir.path() + QDir::separator() + fileName;
        if (QFileInfo::exists(filePath) && QFileInfo(filePath).size() > 1024 &&
            m_settings && m_settings->skipDownloaded())
        {
            continue;
        }

        WDownloadReplyFile *artDownloadReply = new WDownloadReplyFile(m_downloadManager);
        if (!artDownloadReply->setFilePath(filePath)) {
            qWarning() << "Failed to open art file for writing:" << filePath;
            album->setHasErrors(true);
            artDownloadReply->deleteLater();
            continue;
        }

        m_artReplies[album.data()].append(artDownloadReply);

        connect(artDownloadReply, &WDownloadReplyFile::downloadFinished, this,
                [this, album, fileName, artDownloadReply, filePath](QNetworkReply::NetworkError error) {
                    auto artIt = m_artReplies.find(album.data());
                    if (artIt != m_artReplies.end()) {
                        artIt.value().removeOne(artDownloadReply);
                        if (artIt.value().isEmpty()) {
                            m_artReplies.erase(artIt);
                        }
                    }

                    if (error == QNetworkReply::OperationCanceledError)
                    {
                        artDownloadReply->deleteLater();
                        return;
                    }
                    if (error != QNetworkReply::NoError) {
                        album->setHasErrors(true);

                        qWarning() << "Request failed downloading art:" << fileName << "to" << filePath;
                        artDownloadReply->deleteLater();
                        return;
                    }

                    qDebug() << "Successfully downloaded:" << fileName << "to" << filePath;

                    artDownloadReply->deleteLater();
                }, Qt::QueuedConnection);


        m_downloadManager->enqueueDownload(artDownloadReply, downloadLink);
    }
}

void DownloaderController::cancelAlbum(Album *album) {
    if (!album) {
        return;
    }

    // Any in-flight metadata request notices the album is gone from
    // m_addedAlbums when it finishes and discards its result.
    m_albumsBeingFetched.remove(album);

    auto artIt = m_artReplies.find(album);
    if (artIt != m_artReplies.end()) {
        for (WDownloadReplyFile *reply: artIt.value()) {
            if (reply) {
                reply->cancel();
            }
        }
        m_artReplies.erase(artIt);
    }

    for (auto song: album->songs()) {
        if (song) {
            song->cancel();
        }
    }
    album->setIsDownloading(false);
    album->resetState();

    for (auto it = m_addedAlbums.begin(); it != m_addedAlbums.end(); ++it) {
        if (it.key().data() == album) {
            m_addedAlbums.erase(it);
            break;
        }
    }
}

void DownloaderController::retryAlbum(Album *album) {
    if (!album || !album->hasErrors()) {
        return;
    }

    QSharedPointer<Album> albumPtr;
    DownloadQuality quality = DownloadQuality::BEST;

    for (auto it = m_addedAlbums.begin(); it != m_addedAlbums.end(); ++it) {
        if (it.key().data() == album) {
            albumPtr = it.key();
            quality = it.value();
            break;
        }
    }

    if (!albumPtr) {
        qWarning() << "Cannot retry - album not found in managed albums";
        return;
    }

    quality = normalizeQuality(quality);
    album->setHasErrors(false);

    if (!album->isInfoParsed()) {
        album->resetState();
        fetchFullAlbumData(albumPtr);
        return;
    }

    bool hasErroredSongs = false;
    for (auto song: album->songs()) {
        if (!song) {
            continue;
        }

        if (song->errored()) {
            hasErroredSongs = true;
            song->setErrored(false);
            song->setDownloaded(false);

            if (!song->getAllDownloadLinksParsed()) {
                fetchSongDownloadLinks(song, quality, albumPtr);
            } else {
                downloadSongFile(song, quality, albumPtr);
            }
        }
    }


    if (!hasErroredSongs) {
        // The metadata is already parsed, so a full re-fetch would duplicate
        // the song list. Retry the artwork instead; that is the usual cause of
        // an album-level error at this point.
        if (m_settings && m_settings->downloadArt()) {
            processArtDownloads(albumPtr);
        }
    }
}

void DownloaderController::fetchSongDownloadLinks(QSharedPointer<Song> song, DownloadQuality quality,
                                                  QSharedPointer<Album> album) {
    if (!song || !album) {
        return;
    }

    fetchPage(QUrl("https://downloads.khinsider.com" + song->songLink()),
              [this, song, quality, album](QNetworkReply::NetworkError error,
                                           const QByteArray &body) {
                  if (error == QNetworkReply::OperationCanceledError) {
                      return;
                  }

                  if (m_addedAlbums.constFind(album) == m_addedAlbums.constEnd()) {
                      return;
                  }

                  if (error != QNetworkReply::NoError) {
                      album->setHasErrors(true);
                      song->setErrored(true);

                      qWarning() << "Request failed parsing song download links";
                      return;
                  }

                  htmlDocPtr doc = htmlReadMemory(body.constData(), body.size(), nullptr,
                                                  nullptr,
                                                  HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
                  if (!doc) {
                      qWarning() << "HTML read failed while parsing song download links";
                      song->setErrored(true);
                      album->setHasErrors(true);
                      return;
                  }

                  if (!KhinsiderParser::ParseDownloadLink(doc, song)) {
                      song->setErrored(true);
                      album->setHasErrors(true);
                      xmlFreeDoc(doc);
                      return;
                  }
                  xmlFreeDoc(doc);

                  song->setAllDownloadLinksParsed(true);

                  onSongLinksFound(song, quality, album);
              });
}

void DownloaderController::onSongLinksFound(QSharedPointer<Song> song, DownloadQuality quality,
                                            QSharedPointer<Album> album) {
    if (!song || !album) {
        return;
    }
    album->setIsDownloading(true);
    downloadSongFile(song, quality, album);
}

void DownloaderController::downloadSongFile(QSharedPointer<Song> song, DownloadQuality quality,
                                            QSharedPointer<Album> album) {
    if (!song || !album) {
        return;
    }

    quality = normalizeQuality(quality);

    QString downloadLink = "";
    const QMap<QString, QString> links = song->downloadLinks();

    if (quality == DownloadQuality::BEST) {
        int highestRank = 0;
        for (auto it = links.constBegin(); it != links.constEnd(); ++it) {
            int currentRank = qualityRanking.value(it.key(), 0);
            if (currentRank > highestRank) {
                highestRank = currentRank;
                downloadLink = it.value();
            }
        }
    }

    if (downloadLink.isEmpty()) {
        downloadLink = links.value("MP3");
    }

    if (downloadLink.isEmpty() && quality == DownloadQuality::BEST && !links.isEmpty()) {
        downloadLink = links.constBegin().value();
    }

    if (downloadLink.isEmpty()) {
        qWarning() << "No download link found for song:" << song->name();
        song->setErrored(true);
        album->setHasErrors(true);
        return;
    }

    QString downloadPath = m_settings ? m_settings->downloadPath() : QString();
    if (downloadPath.isEmpty()) {
        qWarning() << "Download path is not configured";
        song->setErrored(true);
        album->setHasErrors(true);
        return;
    }

    QString albumName = album->name();
    if (albumName.isEmpty()) {
        albumName = "Unknown Album";
    }

    const QString invalidChars = "\\/:*?\"<>|";
    for (QChar c: invalidChars) {
        albumName.remove(c);
    }

    QDir albumDir(downloadPath + QDir::separator() + albumName);
    if (!albumDir.exists() && !albumDir.mkpath(".")) {
        qWarning() << "Failed to create album directory" << albumDir.path();
        song->setErrored(true);
        album->setHasErrors(true);
        return;
    }

    QUrl url(downloadLink);
    QString fileName = QUrl::fromPercentEncoding(url.path().section('/', -1).toUtf8());

    for (const QChar &c: invalidChars) {
        fileName.replace(c, '_');
    }
    if (fileName.isEmpty()) {
        fileName = song->name();
        for (const QChar &c: invalidChars) {
            fileName.replace(c, '_');
        }
    }
    if (fileName.isEmpty()) {
        fileName = "track";
    }

    QString filePath = albumDir.path() + QDir::separator() + fileName;

    if (QFileInfo(filePath).exists() && QFileInfo(filePath).size() > 1024 &&
        m_settings && m_settings->skipDownloaded()) {
        song->setDownloaded(true);
        emit songDownloadFinished(song);
        return;
    }

    WDownloadReplyFile *songDownloadReply = new WDownloadReplyFile(m_downloadManager);
    if (!songDownloadReply->setFilePath(filePath)) {
        qWarning() << "Failed to open song file for writing:" << filePath;
        song->setErrored(true);
        album->setHasErrors(true);
        songDownloadReply->deleteLater();
        return;
    }

    connect(songDownloadReply, &WDownloadReplyFile::downloadFinished, this,
            [this, song, album, songDownloadReply, filePath](QNetworkReply::NetworkError error) {
                if(error == QNetworkReply::OperationCanceledError)
                {
                    songDownloadReply->deleteLater();
                    return;
                }
                if (error != QNetworkReply::NoError) {
                    song->setErrored(true);
                    album->setHasErrors(true);

                    qWarning() << "Request failed downloading song:" << song->name() << "to" << filePath;
                    songDownloadReply->deleteLater();
                    return;
                }

                qDebug() << "Successfully downloaded:" << song->name() << "to" << filePath;
                song->setDownloaded(true);
                emit songDownloadFinished(song);
                songDownloadReply->deleteLater();
            }, Qt::QueuedConnection);

    connect(songDownloadReply, &WDownloadReplyBase::downloadSpeed,
            this, [song](qint64 speed) {
                song->setDownloadSpeed(speed);
            },
            Qt::QueuedConnection);

    connect(song.get(), &Song::wantToCancel, songDownloadReply, &WDownloadReplyMemory::cancel, Qt::QueuedConnection);
    m_downloadManager->enqueueDownload(songDownloadReply, downloadLink);
}

void DownloaderController::startDownload() {
    for (auto it = m_addedAlbums.begin(); it != m_addedAlbums.end(); ++it) {
        auto album = it.key();
        if (!album) {
            continue;
        }

        DownloadQuality quality = normalizeQuality(it.value());

        if (!album->isInfoParsed()) {
            fetchFullAlbumData(album);
        } else {
            processSongDownloads(album, quality);
        }
    }
}

void DownloaderController::onDownloadListAdd(QSharedPointer<Album> album, DownloadQuality quality) {
    if (!album) {
        return;
    }

    addAlbumToDownload(album, quality);
    updateModelData();

    if (!album->isInfoParsed()) {
        fetchFullAlbumData(album);
    }
    else
    {
        processSongDownloads(album, normalizeQuality(quality));
    }
}

void DownloaderController::updateModelData() {
    QVector<QSharedPointer<Album> > albumsForModel;
    for (auto it = m_addedAlbums.begin(); it != m_addedAlbums.end(); ++it) {
        if (it.key()) {
            albumsForModel.append(it.key());
        }
    }

    m_Model->setAlbums(albumsForModel);
}

int DownloaderController::addToDownloadListImpl(const QString &List) {
    int added = 0;
    QStringList url_list = List.split('\n');
    for (const QString &rawUrl: std::as_const(url_list)) {
        QString url = rawUrl.trimmed();
        if (url.isEmpty()) {
            continue;
        }

        if (!url.startsWith("http://", Qt::CaseInsensitive) &&
            !url.startsWith("https://", Qt::CaseInsensitive) &&
            url.startsWith("downloads.khinsider.com", Qt::CaseInsensitive)) {
            url.prepend("https://");
        }

        bool alreadyAdded = false;
        for (auto it = m_addedAlbums.begin(); it != m_addedAlbums.end(); ++it) {
            QSharedPointer<Album> album = it.key();
            if (album && album->albumLink() == url) {
                alreadyAdded = true;
                break;
            }
        }

        if (alreadyAdded) {
            continue;
        }

        static const QRegularExpression albumre("game-soundtracks/album/(.*)");
        static const QRegularExpression playlistre("/playlist/(.*)");

        auto processMatch = [&](const QRegularExpressionMatch &match, const QString &url, bool isPlaylist) {
            QString albumName = match.captured(1);
            auto newAlbum = QSharedPointer<Album>::create();

            if (isPlaylist) {
                newAlbum->setIsPlaylist(true);
            }

            newAlbum->setName(albumName);
            newAlbum->setAlbumLink(url);
            onDownloadListAdd(newAlbum, normalizeQuality(m_settings ? m_settings->preferredAudioQuality()
                                                                  : DownloadQuality::MP3));
            ++added;
        };

        QRegularExpressionMatch match = albumre.match(url);
        if (match.hasMatch()) {
            processMatch(match, url, false);
            continue;
        }

        match = playlistre.match(url);
        if (match.hasMatch()) {
            processMatch(match, url, true);
        }
    }

    return added;
}

void DownloaderController::addToDownloadList(const QString &List) {
    addToDownloadListImpl(List);
}

int DownloaderController::importUrls(const QString &list) {
    return addToDownloadListImpl(list);
}

void DownloaderController::onThreadsNumberChanged() {
    if (!m_settings) {
        return;
    }
    m_downloadManager->setThreadCount(m_settings->downloadThreads());
}

void DownloaderController::onMaxConcurrentOperationsChanged() {
    if (!m_settings) {
        return;
    }
    m_downloadManager->setDownloadLimitPerThread(m_settings->maxConcurrentDownloadsPerThread());
}

void DownloaderController::requestAddAlbums(QVector<QSharedPointer<Album> > albums, DownloadQuality quality) {
    quality = normalizeQuality(quality);

    for (const auto &album: albums) {
        if (!album) {
            continue;
        }

        addAlbumToDownload(album, quality);

        if (!album->isInfoParsed()) {
            fetchFullAlbumData(album);
        } else {
            processSongDownloads(album, quality);
        }
    }

    updateModelData();
}

void DownloaderController::parseDownloadLinks(QSharedPointer<Album> album) {
    if (!album) {
        return;
    }

    auto qualityIt = m_addedAlbums.constFind(album);
    if (qualityIt == m_addedAlbums.constEnd()) {
        return;
    }

    // unused
    for (QSharedPointer<Song> &song: album->songs()) {
        if (song && !song->getAllDownloadLinksParsed()) {
            fetchSongDownloadLinks(song, qualityIt.value(), album);
        }
    }
}
