#include "DownloaderModel.h"
#include "parser/KhinsiderParser.h"
#include "parser/Album.h"

DownloaderModel::DownloaderModel(QObject *parent) : QAbstractListModel(parent) {
}

int DownloaderModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return m_albums.count();
}

Album *DownloaderModel::getAlbumAt(int index) const {
    if (index < 0 || index >= m_albums.size()) {
        return nullptr;
    }
    return m_albums.at(index).data();
}

QVariant DownloaderModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_albums.count())
        return QVariant();

    const auto &album = m_albums.at(index.row());
    if (!album) {
        return QVariant();
    }

    switch (role) {
        case NameRole: return album->name();
        //case StatusRole: return album->status();
        case ProgressRole: {
            if (album->totalSongs() == 0) return 0;
            return (album->downloadedSongs() * 100) / album->totalSongs();
        };
        case SpeedRole: return album->downloadSpeed();
        case SizeRole: {
            if (album->totalSongs() == 0) return 0;
            return album->downloadedSongs() / album->totalSongs();
        }
        case StateRole: return album->state();
        case DownloadedSongsRole: return album->downloadedSongs();
        case TotalSongsRole: return album->totalSongs();
        default: return QVariant();
    }
}

QHash<int, QByteArray> DownloaderModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[StatusRole] = "status";
    roles[ProgressRole] = "progress";
    roles[SpeedRole] = "speed";
    roles[SizeRole] = "size";
    roles[StateRole] = "state";
    roles[DownloadedSongsRole] = "downloadedSongs";
    roles[TotalSongsRole] = "totalSongs";
    return roles;
}

double DownloaderModel::totalProgress() const {
    const qint64 total = totalSize();
    if (total <= 0) {
        // Sizes are only known once the album pages have been parsed. Until
        // then, fall back to how many files are done.
        const int songs = totalSongs();
        if (songs <= 0) {
            return 0;
        }
        return qBound(0.0, totalDownloadedSongs() * 100.0 / songs, 100.0);
    }
    return qBound(0.0, totalDownloadedSize() * 100.0 / total, 100.0);
}

qint64 DownloaderModel::totalDownloadedSize() const {
    qint64 total = 0;
    for (const auto &album: m_albums) {
        if (album) total += album->downloadedSize();
    }
    return total;
}

qint64 DownloaderModel::totalSpeed() const {
    qint64 total = 0;
    for (const auto &album: m_albums) {
        if (album) total += album->downloadSpeed();
    }
    return total;
}

qint64 DownloaderModel::totalSize() const {
    qint64 total = 0;
    for (const auto &album: m_albums) {
        if (album) total += album->totalSize();
    }
    return total;
}

int DownloaderModel::totalDownloadedSongs() const {
    int total = 0;
    for (const auto &album: m_albums) {
        if (album) total += album->downloadedSongs();
    }
    return total;
}

int DownloaderModel::totalSongs() const {
    int total = 0;
    for (const auto &album: m_albums) {
        if (album) total += album->totalSongs();
    }
    return total;
}

// Slots for canceling
void DownloaderModel::cancelAllDownloads() {
    const QVector<QSharedPointer<Album> > albums = m_albums;

    beginResetModel();
    for (const auto &album: m_albums) {
        if (album) {
            disconnect(album.data(), &Album::progressUpdated, this, nullptr);
            disconnect(album.data(), &Album::stateChanged, this, nullptr);
        }
    }
    m_albums.clear();
    endResetModel();
    emit totalsChanged();

    for (const auto &album: albums) {
        if (album) {
            emit albumCancelRequested(album.data());
        }
    }
}

void DownloaderModel::cancelAlbum(int index) {
    if (index < 0 || index >= m_albums.size()) {
        return;
    }

    const QSharedPointer<Album> album = m_albums.at(index);

    beginResetModel();
    if (album) {
        disconnect(album.data(), &Album::progressUpdated, this, nullptr);
        disconnect(album.data(), &Album::stateChanged, this, nullptr);
    }
    m_albums.removeAt(index);
    endResetModel();
    emit totalsChanged();

    if (album) {
        emit albumCancelRequested(album.data());
    }
    emit totalsChanged();
}

void DownloaderModel::retryAlbum(int index) {
    if (index >= 0 && index < m_albums.size() && m_albums.at(index)) {
        emit albumRetryRequested(m_albums.at(index).data());
    }
}

void DownloaderModel::openAlbumFolder(int index) {
    if (index >= 0 && index < m_albums.size() && m_albums.at(index)) {
        emit albumFolderRequested(m_albums.at(index).data());
    }
}


void DownloaderModel::setAlbums(const QVector<QSharedPointer<Album> > &albums) {
    beginResetModel();

    for (const auto &album: m_albums) {
        if (album) {
            disconnect(album.data(), &Album::progressUpdated, this, nullptr);
            disconnect(album.data(), &Album::stateChanged, this, nullptr);
        }
    }

    m_albums = albums;
    for (const auto &album: m_albums) {
        if (!album) continue;

        Album *albumPtr = album.data();
        auto refresh = [this, albumPtr]() {
            int row = -1;
            for (int i = 0; i < m_albums.size(); ++i) {
                if (m_albums.at(i).data() == albumPtr) {
                    row = i;
                    break;
                }
            }
            if (row >= 0) emit dataChanged(index(row), index(row));
            emit totalsChanged();
        };
        connect(albumPtr, &Album::progressUpdated, this, refresh);
        connect(albumPtr, &Album::stateChanged, this, refresh);
    }
    endResetModel();
    emit totalsChanged();
}

void DownloaderModel::insertAlbum(const QSharedPointer<Album> &album) {
    if (!album) {
        return;
    }

    beginInsertRows(QModelIndex(), m_albums.count(), m_albums.count());
    m_albums.append(album);

    Album *albumPtr = album.data();
    auto refresh = [this, albumPtr]() {
        int idx = -1;
        for (int i = 0; i < m_albums.size(); ++i) {
            if (m_albums.at(i).data() == albumPtr) {
                idx = i;
                break;
            }
        }
        if (idx >= 0) {
            emit dataChanged(index(idx), index(idx));
            emit totalsChanged();
        }
    };
    connect(albumPtr, &Album::progressUpdated, this, refresh);
    connect(albumPtr, &Album::stateChanged, this, refresh);
    endInsertRows();
}
