#ifndef SEARCHRESULTMODEL_H
#define SEARCHRESULTMODEL_H
#include <QAbstractListModel>
#include <QStringList>

#include "parser/Album.h"

class SearchResultModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        DetailsRole,
    };

    explicit SearchResultModel(QObject *parent = nullptr)
        : QAbstractListModel(parent) {
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        return m_albums.count();
    }

    int count() const { return m_albums.count(); }

    QSharedPointer<Album> getAlbumAt(int index) const {
        if (index >= 0 && index < m_albums.count())
            return m_albums.at(index);
        return nullptr;
    }

    // Used by the search UI to check whether a result is already queued.
    Q_INVOKABLE QString albumLinkAt(int index) const {
        const QSharedPointer<Album> album = getAlbumAt(index);
        return album ? album->albumLink() : QString();
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!index.isValid() || index.row() >= m_albums.count())
            return QVariant();

        auto album = m_albums.at(index.row());
        if (!album) {
            return QVariant();
        }

        switch (role) {
            case NameRole:
                return album->name();
            case DetailsRole: {
                QStringList parts;
                if (!album->year().isEmpty()) {
                    parts << album->year();
                }
                if (!album->type().isEmpty()) {
                    parts << album->type();
                }
                const QVector<QString> platforms = album->platforms();
                const QString platformText = QStringList(platforms.begin(), platforms.end()).join(", ");
                if (!platformText.isEmpty()) {
                    parts << platformText;
                }
                return parts.join(QString::fromUtf8(" · "));
            }
        }

        return QVariant();
    }

    QHash<int, QByteArray> roleNames() const override {
        QHash<int, QByteArray> roles;
        roles[NameRole] = "name";
        roles[DetailsRole] = "details";
        return roles;
    }

    void setAlbums(const QVector<QSharedPointer<Album> > &albums) {
        beginResetModel();
        m_albums = albums;
        endResetModel();
        emit countChanged();
    }

    int selectedIndex() const { return m_selectedIndex; }
    QVector<QSharedPointer<Album> > &albums() { return m_albums; }

public slots:
    void onSearchResultsReceived(const QVector<QSharedPointer<Album> > &result) {
        m_selectedIndex = -1;
        setAlbums(result);
        emit selectedIndexChanged();
        emit searchCompleted();
    }

    void onSearchFailed(const QString &message) {
        m_selectedIndex = -1;
        setAlbums({});
        emit selectedIndexChanged();
        emit searchCompleted();
        emit searchFailed(message);
    }

    void setSelectedIndex(int index) {
        if (index < 0 || index >= m_albums.count()) {
            if (m_selectedIndex != -1) {
                m_selectedIndex = -1;
                emit selectedIndexChanged();
            }
            return;
        }

        QSharedPointer<Album> album = getAlbumAt(index);
        if (album) {
            emit albumSelected(album);
        }
        m_selectedIndex = index;
        emit selectedIndexChanged();
    }

signals:
    void searchStarted();

    void searchCompleted();

    void performSearch(const QString &query);

    void requestAddAlbums(QVector<QSharedPointer<Album> > albums, DownloadQuality quality);

    void selectedIndexChanged();

    void countChanged();

    void albumSelected(QSharedPointer<Album> album);

    void searchFailed(const QString &message);

private:
    QVector<QSharedPointer<Album> > m_albums;
    int m_selectedIndex = -1;
};
#endif //SEARCHRESULTMODEL_H
