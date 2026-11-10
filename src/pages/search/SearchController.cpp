#include "SearchController.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QQmlApplicationEngine>

#include "ImagePrecache.h"
#include "KhinsiderRequest.h"
#include "parser/KhinsiderParser.h"

#define SEARCH_URL "https://downloads.khinsider.com/search?search="
#include "parser/Album.h"

SearchController::SearchController(QObject *parent) {
    networkManager = new QNetworkAccessManager(this);
    m_albumInfoVM = new SearchAlbumInfoViewModel(this);
    m_searchResultVM = new SearchResultModel(this);

    connect(m_searchResultVM, &SearchResultModel::albumSelected,
            this, &SearchController::fetchFullAlbumData);
    connect(m_searchResultVM, &SearchResultModel::albumSelected,
            m_albumInfoVM, &SearchAlbumInfoViewModel::setCurrentAlbum);
    // A new search or a failed one resets the selection; the info sidebar must
    // not keep showing the previous album.
    connect(m_searchResultVM, &SearchResultModel::selectedIndexChanged, this, [this]() {
        if (m_searchResultVM->selectedIndex() < 0) {
            m_albumInfoVM->setCurrentAlbum(QSharedPointer<Album>());
        }
    });
    connect(this, &SearchController::searchResultsReceived,
            m_searchResultVM, &SearchResultModel::onSearchResultsReceived);
    connect(m_searchResultVM, &SearchResultModel::performSearch,
            this, &SearchController::doSearch);
    connect(this, &SearchController::searchStarted,
            m_searchResultVM, &SearchResultModel::searchStarted);
    connect(this, &SearchController::searchFailed,
            m_searchResultVM, &SearchResultModel::onSearchFailed);
    connect(this, &SearchController::onFullFetched,
            m_albumInfoVM, &SearchAlbumInfoViewModel::onFullFetch);
    connect(m_albumInfoVM, &SearchAlbumInfoViewModel::downloadRequested, this, &SearchController::onDownloadAddRequest);
    connect(m_searchResultVM, &SearchResultModel::requestAddAlbums, this, &SearchController::onDownloadAllAddRequest);
}

void SearchController::doSearch(const QString &query) {
    // Ignore the response of a previous search that is still in flight.
    if (m_searchReply) {
        disconnect(m_searchReply, nullptr, this, nullptr);
        m_searchReply->abort();
        m_searchReply->deleteLater();
        m_searchReply = nullptr;
    }

    emit searchStarted();
    QUrl url(SEARCH_URL + QUrl::toPercentEncoding(query) + "&albumListSize=compact");
    QNetworkRequest request = khinsiderRequest(url);
    QNetworkReply *reply = networkManager->get(request);
    m_searchReply = reply;

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply != m_searchReply) {
            reply->deleteLater();
            return;
        }
        m_searchReply = nullptr;

        QVector<QSharedPointer<Album> > res;

        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "Search request failed:" << reply->errorString();
            const QString message = describeKhinsiderError(reply);
            reply->deleteLater();
            emit searchFailed(message);
            return;
        }

        const QByteArray html = reply->readAll();
        reply->deleteLater();

        htmlDocPtr doc = htmlReadMemory(html.constData(), html.size(), nullptr, nullptr,
                                        HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
        if (!doc)
        {
            qWarning() << "HTML read failed";
            emit searchResultsReceived(res);
            return;
        }

        KhinsiderParser::ParseSearchResults(doc, res);
        xmlFreeDoc(doc);
        emit searchResultsReceived(res);
    });
}

// Parsing lives in KhinsiderParser so the downloader and this preview panel
// stay in sync.
void SearchController::fetchFullAlbumData(QSharedPointer<Album> album) {
    if (!album || album->isInfoParsed()) {
        return;
    }

    if (m_albumReplies.contains(album.data())) {
        return;
    }

    QUrl url(album->albumLink());
    QNetworkRequest request = khinsiderRequest(url);
    QNetworkReply *reply = networkManager->get(request);
    m_albumReplies.insert(album.data(), reply);

    connect(reply, &QNetworkReply::finished, this, [this, reply, album]() {
        if (m_albumReplies.value(album.data()) != reply) {
            reply->deleteLater();
            return;
        }
        m_albumReplies.remove(album.data());

        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "Album request failed:" << reply->errorString();
            album->setHasErrors(true);
            reply->deleteLater();
            return;
        }

        const QByteArray html = reply->readAll();
        reply->deleteLater();

        if (!KhinsiderParser::ParseAlbumPageHtml(html, album)) {
            album->setHasErrors(true);
            return;
        }

        ImagePrecache::instance()->precache(album->albumImage());
        emit onFullFetched();
    });
}
