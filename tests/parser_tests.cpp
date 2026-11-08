#include <QtTest>
#include <QSharedPointer>
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>

#include "parser/Album.h"
#include "parser/HTMLParserHelper.h"
#include "parser/KhinsiderParser.h"
#include "parser/Song.h"

class ParserTests : public QObject {
    Q_OBJECT

private slots:
    void parseDownloadLinkCollectsAudioLinks();
    void parseDownloadLinkUsesFallbackLayout();
    void parseDownloadLinkReturnsFalseWithoutAudioLinks();
    void toNextTagHandlesNull();
    void skipToFindsLastNode();
    void sizeHelpers();
};

namespace {
htmlDocPtr readHtml(const QByteArray &html) {
    return htmlReadMemory(html.constData(), html.size(), nullptr, nullptr,
                          HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
}
}

void ParserTests::parseDownloadLinkCollectsAudioLinks() {
    const QByteArray html = R"(
        <html><body>
          <div id="pageContent">
            <p>one</p><p>two</p><p>three</p>
            <p>
              <a href="https://example.com/song.mp3">MP3</a>
              <a href="https://example.com/song.flac?token=1">FLAC</a>
              <a href="https://example.com/info.html">Info</a>
            </p>
          </div>
        </body></html>
    )";

    htmlDocPtr doc = readHtml(html);
    QVERIFY(doc);

    auto song = QSharedPointer<Song>::create();
    QVERIFY(KhinsiderParser::ParseDownloadLink(doc, song));

    QCOMPARE(song->downloadLinks().value("MP3"), QString("https://example.com/song.mp3"));
    QCOMPARE(song->downloadLinks().value("FLAC"), QString("https://example.com/song.flac?token=1"));
    QCOMPARE(song->downloadLinks().size(), 2);

    xmlFreeDoc(doc);
}

void ParserTests::parseDownloadLinkUsesFallbackLayout() {
    const QByteArray html = R"(
        <html><body>
          <div id="pageContent">
            <p><a href="https://example.com/song.ogg">OGG</a></p>
          </div>
        </body></html>
    )";

    htmlDocPtr doc = readHtml(html);
    QVERIFY(doc);

    auto song = QSharedPointer<Song>::create();
    QVERIFY(KhinsiderParser::ParseDownloadLink(doc, song));
    QCOMPARE(song->downloadLinks().value("OGG"), QString("https://example.com/song.ogg"));

    xmlFreeDoc(doc);
}

void ParserTests::parseDownloadLinkReturnsFalseWithoutAudioLinks() {
    const QByteArray html = R"(
        <html><body>
          <div id="pageContent">
            <p>one</p><p>two</p><p>three</p>
            <p><a href="https://example.com/info.html">Info</a></p>
          </div>
        </body></html>
    )";

    htmlDocPtr doc = readHtml(html);
    QVERIFY(doc);

    auto song = QSharedPointer<Song>::create();
    QVERIFY(!KhinsiderParser::ParseDownloadLink(doc, song));
    QVERIFY(song->downloadLinks().isEmpty());

    xmlFreeDoc(doc);
}

void ParserTests::toNextTagHandlesNull() {
    QCOMPARE(HTMLParserHelper::ToNextTag(nullptr, "p"), static_cast<xmlNodePtr>(nullptr));
    QCOMPARE(HTMLParserHelper::GetNextCount(nullptr), 0);
}

void ParserTests::skipToFindsLastNode() {
    const QByteArray html = R"(
        <html><body><div><span>one</span><span>Target</span></div></body></html>
    )";

    htmlDocPtr doc = readHtml(html);
    QVERIFY(doc);

    xmlXPathContextPtr context = xmlXPathNewContext(doc);
    QVERIFY(context);

    xmlXPathObjectPtr result = xmlXPathEvalExpression(
        reinterpret_cast<const xmlChar *>("//span[1]"), context);
    QVERIFY(result);
    QVERIFY(result->nodesetval);
    QCOMPARE(result->nodesetval->nodeNr, 1);

    xmlNodePtr firstSpan = result->nodesetval->nodeTab[0];
    xmlNodePtr found = HTMLParserHelper::SkipTo("Target", firstSpan);
    QVERIFY(found);

    xmlChar *content = xmlNodeGetContent(found);
    QVERIFY(content);
    QCOMPARE(QString::fromUtf8(reinterpret_cast<const char *>(content)), QString("Target"));
    xmlFree(content);

    xmlXPathFreeObject(result);
    xmlXPathFreeContext(context);
    xmlFreeDoc(doc);
}

void ParserTests::sizeHelpers() {
    QCOMPARE(HTMLParserHelper::convertToKB("1.5 MB"), 1536UL);
    QCOMPARE(HTMLParserHelper::convertToKB("2 GB"), 2097152UL);
    QCOMPARE(HTMLParserHelper::convertToKB("512 KB"), 512UL);
    QCOMPARE(HTMLParserHelper::convertToKB("garbage"), 0UL);

    QCOMPARE(HTMLParserHelper::convertToSeconds("3:45"), 225);
    QCOMPARE(HTMLParserHelper::convertToSeconds("1:02:03"), 3723);
    QCOMPARE(HTMLParserHelper::convertToSeconds("bad"), 0);
}

QTEST_MAIN(ParserTests)
#include "parser_tests.moc"
