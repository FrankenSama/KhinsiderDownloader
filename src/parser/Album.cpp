#include "Album.h"

void Album::resetState() {
    m_isDownloading = false;
    m_hasErrors = false;
    for (const auto &song: m_songs) {
        if (!song) {
            continue;
        }
        song->setDownloaded(false);
        song->setDownloadedSize(0);
        song->setDownloadSpeed(0);
        song->setErrored(false);
    }
}
