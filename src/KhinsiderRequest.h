#ifndef KHINSIDERREQUEST_H
#define KHINSIDERREQUEST_H

#include <QFile>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QString>
#include <QUrl>

// downloads.khinsider.com is behind a Cloudflare managed challenge that rejects
// requests without a User-Agent. Qt does not send one by default, so every
// request to the site used to come back as HTTP 403 "Just a moment...".
//
// Any identifying User-Agent is accepted (verified against search, album and
// song pages), so keep it honest and specific to this application instead of
// impersonating a browser.
inline QByteArray khinsiderUserAgent() {
    static const QByteArray userAgent = [] {
        QString version;
        QFile versionFile(QStringLiteral(":/APP_VERSION"));
        if (versionFile.open(QIODevice::ReadOnly)) {
            version = QString::fromUtf8(versionFile.readAll()).trimmed();
        }

        if (version.isEmpty()) {
            return QByteArrayLiteral("KhinsiderDownloader");
        }

        return QStringLiteral("KhinsiderDownloader/%1").arg(version).toUtf8();
    }();

    return userAgent;
}

inline void applyKhinsiderHeaders(QNetworkRequest &request) {
    request.setHeader(QNetworkRequest::UserAgentHeader, khinsiderUserAgent());

    if (!request.hasRawHeader("Accept")) {
        request.setRawHeader("Accept",
                             "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    }
}

inline QNetworkRequest khinsiderRequest(const QUrl &url) {
    QNetworkRequest request(url);
    applyKhinsiderHeaders(request);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);
    return request;
}

// Turns a failed reply into something a user can act on. A 403 from this site
// is a Cloudflare challenge rather than a real permission problem, and the raw
// error string ("server replied with status code 403") only confuses people.
inline QString describeKhinsiderError(QNetworkReply *reply) {
    if (!reply) {
        return QStringLiteral("Unknown network error");
    }

    const QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (statusCode.isValid() && statusCode.toInt() == 403) {
        return QStringLiteral(
            "Khinsider is refusing automated requests right now (HTTP 403). "
            "This is usually a temporary Cloudflare check - try again in a minute.");
    }

    return reply->errorString();
}

#endif // KHINSIDERREQUEST_H
