#ifndef DOWNLOADERMODEL_H
#define DOWNLOADERMODEL_H
#include <QAbstractListModel>
#include <QSharedPointer>
#include <QTimer>
#include "parser/Album.h"


class DownloaderModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        StatusRole,
        ProgressRole,
        SpeedRole,
        SizeRole,
        StateRole,
        DownloadedSongsRole,
        TotalSongsRole,
        DownloadedSizeRole,
        TotalSizeRole,
        ErrorRole
    };

    explicit DownloaderModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QHash<int, QByteArray> roleNames() const override;

    Album *getAlbumAt(int index) const;

    Q_INVOKABLE double totalProgress() const;

    Q_INVOKABLE qint64 totalDownloadedSize() const;

    Q_INVOKABLE qint64 totalSpeed() const;

    Q_INVOKABLE qint64 totalSize() const;

    Q_INVOKABLE int totalDownloadedSongs() const;

    Q_INVOKABLE int totalSongs() const;

    // Number of finished albums currently in the queue, used by the bulk
    // "clear completed" action.
    Q_INVOKABLE int completedCount() const;

    // Number of albums that failed and can be retried.
    Q_INVOKABLE int failedCount() const;

    // Opens the folder this album was downloaded into.
    Q_INVOKABLE void openAlbumFolder(int index);

public slots:
    void cancelAllDownloads();

    void cancelAlbum(int index);

    void retryAlbum(int index);

    void setAlbums(const QVector<QSharedPointer<Album> > &albums);

    void insertAlbum(const QSharedPointer<Album> &album);

signals:
    void albumCancelRequested(Album *album);

    void albumRetryRequested(Album *album);

    void albumFolderRequested(Album *album);

    void totalsChanged();

    void addToDownloadList(const QString &List);

private:
    // Coalesces the expensive queue-wide totals recomputation. Per-row updates
    // stay immediate, but a busy queue can emit progress many times a second.
    void scheduleTotalsChanged();

    QVector<QSharedPointer<Album> > m_albums;
    QTimer m_totalsTimer;
    bool m_totalsPending = false;
};

#endif // DOWNLOADERMODEL_H
