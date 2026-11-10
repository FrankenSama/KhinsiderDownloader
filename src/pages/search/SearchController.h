#ifndef SEARCHCONTROLLER_H
#define SEARCHCONTROLLER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QHash>
#include <QString>

#include "SearchAlbumInfoViewModel.h"
#include "SearchResultModel.h"
#include "parser/KhinsiderParser.h"

class Album;
class QNetworkReply;

class SearchController : public QObject {
    Q_OBJECT

    Q_PROPERTY(SearchAlbumInfoViewModel* albumInfoVM READ albumInfoVM CONSTANT)
    Q_PROPERTY(SearchResultModel* searchResultVM READ searchResultVM CONSTANT)

public:
    SearchAlbumInfoViewModel *albumInfoVM() { return m_albumInfoVM; }
    SearchResultModel *searchResultVM() { return m_searchResultVM; }

    explicit SearchController(QObject *parent = nullptr);

    void fetchFullAlbumData(QSharedPointer<Album> album);

signals:
    void onDownloadAllAddRequest(QVector<QSharedPointer<Album> > Albums, DownloadQuality quality);

    void onDownloadAddRequest(QSharedPointer<Album> Album, DownloadQuality quality);

    void searchResultsReceived(const QVector<QSharedPointer<Album> > &result);

    void onFullFetched();

    void searchStarted();

    // Emitted when the search request itself fails (network error), so the UI
    // can distinguish "no matches" from "could not reach the site".
    void searchFailed(const QString &message);

public slots:
    void doSearch(const QString &text);

    // Queues every current search result and returns how many were queued, so
    // the caller can confirm the action instead of silently clearing the list.
    Q_INVOKABLE int addAllAlbumsToDownloads() {
        const int count = m_searchResultVM->albums().count();
        emit onDownloadAllAddRequest(m_searchResultVM->albums(), DownloadQuality::UNKNOWN);
        return count;
    }

    // Queues a single search result and returns its name, or an empty string
    // when the index is out of range. Used by Enter in the results list.
    Q_INVOKABLE QString addAlbumToDownloads(int index) {
        const QSharedPointer<Album> album = m_searchResultVM->getAlbumAt(index);
        if (!album) {
            return QString();
        }
        emit onDownloadAddRequest(album, DownloadQuality::UNKNOWN);
        return album->name();
    }

private:
    QNetworkAccessManager *networkManager;
    SearchResultModel *m_searchResultVM;
    SearchAlbumInfoViewModel *m_albumInfoVM;
    QNetworkReply *m_searchReply = nullptr;
    QHash<Album *, QNetworkReply *> m_albumReplies;
};

#endif //SEARCHCONTROLLER_H
