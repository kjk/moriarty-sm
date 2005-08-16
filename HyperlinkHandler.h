#ifndef INFOMAN_HYPERLINK_HANDLER_H__
#define INFOMAN_HYPERLINK_HANDLER_H__

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
};

void HyperlinkHandlerDispose();

#endif