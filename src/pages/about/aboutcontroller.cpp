#include "aboutcontroller.h"
#include <qdir.h>
#include <qversionnumber.h>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <algorithm>

AboutController::AboutController(QObject *parent)
    : QObject(parent) {
    m_version = "0.0.0.0";
    QFile file(":/APP_VERSION");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        m_version = in.readLine().trimmed();
    }
    checkForUpdates();
}

QString AboutController::appVersion() const {
    return m_version;
}

namespace {
QString normalizedVersion(const QString &version) {
    QString normalized = version.trimmed();
    if (normalized.startsWith('v', Qt::CaseInsensitive)) {
        normalized.remove(0, 1);
    }
    const int suffixIndex = normalized.indexOf('-');
    if (suffixIndex >= 0) {
        normalized = normalized.left(suffixIndex);
    }
    return normalized;
}
}

bool AboutController::isNewVersion(const QString &latestVersion, const QString &currentVersion) {
    QVersionNumber currentVer = QVersionNumber::fromString(normalizedVersion(currentVersion));
    QVersionNumber latestVer = QVersionNumber::fromString(normalizedVersion(latestVersion));

    // Compare the version numbers
    return latestVer > currentVer;
}

void AboutController::checkForUpdates() {
    QString currentVersion = m_version;

    QNetworkRequest request(QUrl("https://api.github.com/repos/FrankenSama/KhinsiderDownloader/releases"));
    request.setHeader(QNetworkRequest::UserAgentHeader, "KhinsiderDownloader");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &AboutController::onUpdateCheckFinished);
    manager->get(request);
}

void AboutController::onUpdateCheckFinished(QNetworkReply *reply) {
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Failed to check for updates!";
        reply->deleteLater();
        return;
    }

    QByteArray response = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(response);
    if (!doc.isArray()) {
        qDebug() << "Update check returned an unexpected response";
        reply->deleteLater();
        return;
    }
    QJsonArray releases = doc.array();

    QString currentVersion = m_version;

    QList<QJsonObject> newerReleases;

    for (const QJsonValue &value: releases) {
        QJsonObject release = value.toObject();
        QString tagName = release["tag_name"].toString().trimmed();
        if (tagName.isEmpty()) {
            continue;
        }

        bool isPrerelease = release["prerelease"].toBool();

        if (!isPrerelease && isNewVersion(tagName, currentVersion)) {
            newerReleases.append(release);
        }
    }

    // Sort newest first; tag names may have a leading "v" and a suffix.
    std::sort(newerReleases.begin(), newerReleases.end(), [](const QJsonObject &a, const QJsonObject &b) {
        return QVersionNumber::fromString(normalizedVersion(a["tag_name"].toString())) >
               QVersionNumber::fromString(normalizedVersion(b["tag_name"].toString()));
    });

    if (!newerReleases.isEmpty()) {
        QJsonObject latestRelease = newerReleases.first();
        QString latestVersion = latestRelease["tag_name"].toString();

        emit foundNewUpdate(latestVersion);
    } else {
        //We're ahead of release!
    }

    reply->deleteLater();
}
