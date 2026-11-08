#include "App.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QNetworkDiskCache>
#include <QStandardPaths>
#include <QQmlNetworkAccessManagerFactory>
#include "curl/curl.h"
#include "ImagePrecache.h"
#include "LogController.h"

class NetworkManagerFactory : public QQmlNetworkAccessManagerFactory {
public:
    QNetworkAccessManager *create(QObject *parent) override {
        QNetworkAccessManager *manager = new QNetworkAccessManager(parent);
        QNetworkDiskCache *diskCache = new QNetworkDiskCache(manager);
        diskCache->setCacheDirectory(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
        manager->setCache(diskCache);
        return manager;
    }
};

int main(int argc, char *argv[]) {
    if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
        return -1;
    }

    QCoreApplication::setOrganizationName("Weespin");
    QCoreApplication::setApplicationName("KhinsiderDownloader");

    QGuiApplication app(argc, argv);
    qmlRegisterType<Settings>("App.Models", 1, 0, "Settings");
    qmlRegisterType<Album>("App.Models", 1, 0, "Album");
    qmlRegisterType<SearchAlbumInfoViewModel>("App.ViewModels", 1, 0, "AlbumInfoViewModel");
    qmlRegisterType<SettingsViewModel>("App.ViewModels", 1, 0, "SettingsViewModel");
    qmlRegisterType<DownloaderModel>("App.Models", 1, 0, "DownloaderModel");

    int result = 0;
    {
        // App must be destroyed before curl_global_cleanup(), because the
        // download workers and their libcurl handles are owned by it.
        App kApp;

        // The factory has to outlive the engine that references it.
        NetworkManagerFactory networkManagerFactory;
        QQmlApplicationEngine engine;
        QNetworkAccessManager *globalManager = networkManagerFactory.create(&engine);
        engine.setNetworkAccessManagerFactory(&networkManagerFactory);

        //Move it somewhere, wtf it's doing here
        auto *precache = new ImagePrecache(globalManager, &engine);

        engine.rootContext()->setContextProperty("app", &kApp);
        QObject::connect(
            &engine,
            &QQmlApplicationEngine::objectCreationFailed,
            &app,
            []() { QCoreApplication::exit(-1); },
            Qt::QueuedConnection);

        engine.loadFromModule("KhinsiderQT", "Main");
        result = app.exec();
    }

    curl_global_cleanup();

    return result;
}
