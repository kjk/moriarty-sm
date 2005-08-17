#ifndef INFOMAN_HYPERLINK_HANDLER_H__
#define INFOMAN_HYPERLINK_HANDLER_H__

#include "InfoManGlobals.h"
#include <HyperlinkHandlerBase.hpp>
#include <PediaUrl.hpp>

#define urlSchemaHttp                         "http"
#define urlSchemaDream                        "dream"
#define urlSchemaMovie                        "movie"
#define urlSchemaTheatre                      "theatre"
#define urlSchemaLyricsForm                   "lyricsform"
#define urlSchemaMenu                         "menu"
#define urlSchemaRunModule                    "runmodule"

#define urlSchemaAmazonPreferences            "amazonpreferences"
#define urlSchemaAmazonForm                   "amazonform"
#define urlSchemaAmazonSearch               "s+amazonsearch"

#define urlSchemaSimpleFormWithDefinition     "simpleform"
#define urlSchemaNetflixForm                  "netflixform"
#define urlSchemaListsOfBestsForm             "listsofbestsform"

#define urlSchemaEncyclopediaTerm             pediaUrlTerm
#define urlSchemaEncyclopediaRandom           pediaUrlRandom
#define urlSchemaEncyclopediaSearch           pediaUrlSearch
#define urlSchemaEncyclopediaLangs            pediaUrlLangs
#define urlSchemaEncyclopediaStats            pediaUrlStats

#define urlSchemaEncyclopedia                 "pedia"

#define urlSchemaDict                         "dict"
#define urlSchemaDictTerm                     "s+dictterm"
#define urlSchemaDictRandom                   "s+dictrandom"
#define urlSchemaDictForm                     "dictform"

#define urlSchemaEBookSearch            "s+eBook-search"
#define urlSchemaEBookDownload          "s+eBook-download"
#define urlSchemaEBookBrowse            "s+eBook-browse"
#define urlSchemaEBookHome              "s+eBook-home"
#define urlSchemaEBook                      "eBook"

#define urlSchemaEBayForm                     "ebayform"

#define urlSchemaClipboardCopy                "clipbrdcopy"

#define urlSeparatorSchema                    ':'
#define urlSeparatorFlags                     '+'
#define urlSeparatorSchemaStr                 ":"
#define urlSeparatorFlagsStr                  "+"

#define urlFlagServer                         's'
#define urlFlagClosePopup                   'c'
#define urlFlagHistory                        'h'
#define urlFlagHistoryInCache                 'H'

#define pediaUrlPartSetLang                   "lang"
#define pediaUrlPartHome                      "home"
#define pediaUrlPartSearchDialog              "search"
#define pediaUrlPartShowArticle               "article"

#define ebookUrlPartSearch "search"
#define ebookUrlPartMove "move"
#define ebookUrlPartDelete "delete"
#define ebookUrlPartCopy "copy"
#define ebookUrlPartLaunch "launch"
#define ebookUrlPartManage "manage"
#define ebookUrlPartBrowse "browse"
#define ebookUrlPartDownload "download"

#define urlSchemaFlickr         "flickr"

#define flickrUrlPartAbout      "about"


class HyperlinkHandler: public HyperlinkHandlerBase {

public:

    typedef void (HyperlinkHandler::* HandlerFunction)(const char* hyperlink, ulong_t len, const Point*);

private:

    static HandlerFunction findHandler(const char* schema, ulong_t len);
    
/*    
    static void closePopup(uint_t id);

    static void closePopup();
 */    
        
    enum HandlerFlag {
        flagServerHyperlink = 1,
        flagClosePopupForm = 2,
        flagHistory = 4,
        flagHistoryInCache = 8
    };
    
    static uint_t interpretFlag(char flag);


public:

    void handleHyperlink(const char* link, ulong_t len, const Point* point);

    ~HyperlinkHandler();

private:
    void handleRunModule(const char* hyperlink, ulong_t len, const Point* point);
    void handleHttp(const char* hyperlink, ulong_t len, const Point* point);
    void handleDream(const char* hyperlink, ulong_t len, const Point* point);
    void handleMovie(const char* hyperlink, ulong_t len, const Point* point);
    void handleTheatre(const char* hyperlink, ulong_t len, const Point* point);
    void handleLyricsForm(const char* hyperlink, ulong_t len, const Point* point);
    void handleDictForm(const char* hyperlink, ulong_t len, const Point* point);
    void handleMenu(const char* hyperlink, ulong_t len, const Point* point);
    void handleAmazonForm(const char* hyperlink, ulong_t len, const Point* point);
    void handleAmazonPreferences(const char* hyperlink, ulong_t len, const Point* point);
    void handleSimpleFormWithDefinition(const char* hyperlink, ulong_t len, const Point* point);
    void handleClipboardCopy(const char* hyperlink, ulong_t len, const Point* point);
    void handleListsOfBestsForm(const char* hyperlink, ulong_t len, const Point* point);
    void handleEBayForm(const char* hyperlink, ulong_t len, const Point* point);
    void handlePedia(const char* hyperlink, ulong_t len, const Point* point);
    void handlePediaHome(const char* hyperlink, ulong_t len, const Point* point);
    void handlePediaLang(const char* hyperlink, ulong_t len, const Point* point);
    void handlePediaSearch(const char* hyperlink, ulong_t len, const Point* point);
    void handlePediaArticle(const char* hyperlink, ulong_t len, const Point* point);
    void handleEBook(const char* hyperlink, ulong_t len, const Point* point);
    void handleEBookDownload(const char* data, ulong_t len);
    void handleNetflixForm(const char* hyperlink, ulong_t len, const Point* point);
    void handleFlickr(const  char* hyperlink, ulong_t len, const Point* point);
};

const char* hyperlinkData(const char* link,  ulong_t& length);

void HyperlinkHandlerDispose();

#endif