#include "InfoManGlobals.h"
#include "HyperlinkHandler.h"
#include "LookupManager.h"
#include <Text.hpp>
#include <SysUtils.hpp>

static HyperlinkHandler* hyperlinkHandler = NULL;

void HyperlinkHandlerDispose() 
{
    delete hyperlinkHandler;
    hyperlinkHandler = NULL; 
}

HyperlinkHandler* GetHyperlinkHandler()
{
    if (NULL == hyperlinkHandler)
        hyperlinkHandler = new_nt HyperlinkHandler();
    return hyperlinkHandler;  
}

HyperlinkHandler::~HyperlinkHandler()
{
}

struct DispatchTableEntry {
    const char* schema;
    HyperlinkHandler::HandlerFunction function;
};

HyperlinkHandler::HandlerFunction HyperlinkHandler::findHandler(const char* schema, ulong_t len)
{
    static const DispatchTableEntry dispatchTable[] =
    {
        {urlSchemaDream, &HyperlinkHandler::handleDream},
        {urlSchemaRunModule, &HyperlinkHandler::handleRunModule},
        {urlSchemaHttp, &HyperlinkHandler::handleHttp},
        {urlSchemaMenu, &HyperlinkHandler::handleMenu},
        {urlSchemaMovie, &HyperlinkHandler::handleMovie},
        {urlSchemaEncyclopedia, &HyperlinkHandler::handlePedia},
        {urlSchemaTheatre, &HyperlinkHandler::handleTheatre},
        {urlSchemaLyricsForm, &HyperlinkHandler::handleLyricsForm},
        {urlSchemaDictForm, &HyperlinkHandler::handleDictForm},
        {urlSchemaAmazonPreferences, &HyperlinkHandler::handleAmazonPreferences},
        {urlSchemaAmazonForm, &HyperlinkHandler::handleAmazonForm},
        {urlSchemaEBayForm, &HyperlinkHandler::handleEBayForm},
        {urlSchemaNetflixForm, &HyperlinkHandler::handleNetflixForm},
        {urlSchemaSimpleFormWithDefinition, &HyperlinkHandler::handleSimpleFormWithDefinition},
        {urlSchemaClipboardCopy, &HyperlinkHandler::handleClipboardCopy},
        {urlSchemaListsOfBestsForm, &HyperlinkHandler::handleListsOfBestsForm},
        {urlSchemaEBook, &HyperlinkHandler::handleEBook},
        {urlSchemaFlickr, &HyperlinkHandler::handleFlickr},
    };
    for (int i = 0; i < ARRAY_SIZE(dispatchTable); ++i)
    {
        if (StrEquals(schema, len, dispatchTable[i].schema))
            return dispatchTable[i].function;
    }
    return NULL;
}

uint_t HyperlinkHandler::interpretFlag(char flag)
{
    switch (flag)
    {
        case urlFlagServer:
            return flagServerHyperlink;
        
        case urlFlagClosePopup:
            return flagClosePopupForm;
            
        case urlFlagHistory:
            return flagHistory;

        case urlFlagHistoryInCache:
            return flagHistoryInCache;
    }
    assert(false);
    return 0;
}


void HyperlinkHandler::handleHyperlink(const char* hyperlink, ulong_t len, const Point* point)
{
    const char* schema = hyperlink;
    long schemaLen = StrFind(hyperlink, len, urlSeparatorSchema);
    if (-1 == schemaLen)
        schemaLen = len;
    
    uint_t flags = 0;
    
    long flagPos = StrFind(schema, schemaLen, urlSeparatorFlags);
    if (-1 != flagPos)
    {
        for (long i = 0; i < flagPos; ++i)
            flags |= interpretFlag(schema[i]);
    }

    // TODO: write WinCE version of closePopup()
/*    
    if (flagClosePopupForm & flags)
    {
        closePopup();
    }
 */    
    if (flagServerHyperlink & flags)
    {
        LookupManager* lm = GetLookupManager();
        lm->fetchUrl(hyperlink);
    }
    else
    {
        HandlerFunction function = findHandler(schema, schemaLen);
        if (NULL == function)
        {
            assert(false);
            return;
        }
        (this->*function)(hyperlink, len, point);
    }
}


void HyperlinkHandler::handleHttp(const char* hyperlink, ulong_t len, const Point*)
{
    assert('\0' == hyperlink[len]);
#ifdef _PALM_OS
    if (errNone != WebBrowserCommand(false, 0, sysAppLaunchCmdGoToURL, hyperlink, NULL))
        FrmAlert(noWebBrowserAlert);    
#endif

#ifdef _WIN32
    if (!OpenURL(hyperlink))
    {
        // TODO: add alert
        assert(false); 
    }  
#endif     
}

const char* hyperlinkData(const char* link,  ulong_t& length)
{
    assert('\0' == link[length]);
    long schemaLen = StrFind(link, length, urlSeparatorSchema);
    if (-1 == schemaLen)
    {
        link += length;
        length = 0;
    }
    else
    {
        link += (schemaLen + 1);
        length -= (schemaLen + 1);
        strip(link, length);
    }
    return link;
}

void HyperlinkHandler::handleDictForm(const char* link,  ulong_t length, const Point*)
{
    //const char* data = hyperlinkData(link, length);
    //MoriartyApplication& app = MoriartyApplication::instance();
    //DictMainForm* form = static_cast<DictMainForm*>(app.getOpenForm(dictMainForm));

    //if (StrEquals("main", data))
    //{
    //    form->showMain();
    //    return;
    //}

    //if (StrEquals("search", data))
    //{
    //    form->handleSearch();
    //    return;
    //}
    //
    //if (StrEquals("random", data))
    //{
    //    form->randomWord();
    //    return;
    //}

    //if (StrEquals("history", data))
    //{
    //    form->handleHistory();
    //    return;
    //}

    //assert(false);
}

void HyperlinkHandler::handleLyricsForm(const char* link,  ulong_t length, const Point*)
{
    //const char* data = hyperlinkData(link, length);
    //if (StrEquals("search", data))
    //{
    //    MoriartyApplication::popupForm(lyricsSearchForm);
    //    return;
    //}

    //if (StrEquals("main", data))
    //{
    //    MoriartyApplication& app = MoriartyApplication::instance();
    //    Lyrics2MainForm* form = static_cast<Lyrics2MainForm*>(app.getOpenForm(lyrics2MainForm));
    //    form->showStartText();
    //    return;
    //}

    //assert(false);
}

//struct MovieTitleEquals {
//    const ArsLexis::String& title;
//    explicit MovieTitleEquals(const ArsLexis::String& t): title(t) {}
//    bool operator()(const Movie* m) const
//    {return m->title == title;}
//};

void HyperlinkHandler::handleMovie(const char* link,  ulong_t length, const Point*)
{
    //MoriartyApplication& app = MoriartyApplication::instance();
    //MoviesMainForm* form = static_cast<MoviesMainForm*>(app.getOpenForm(moviesMainForm));
    //if (NULL == form || NULL == form->theatres)
    //{
    //    assert(false);
    //    return;
    //}
    //const char* data = hyperlinkData(link, length);
    //Movies_t& movies = form->movies;
    //const UniversalDataFormat& theatres = *form->theatres;
    //if (movies.empty() && !theatres.empty())
    //    createMoviesFromTheatres(movies, theatres);
    //uint_t movie=std::find_if(movies.begin(), movies.end(), MovieTitleEquals(String(data, length))) - movies.begin();
    //sendEvent(MoriartyApplication::appSelectMovieEvent, MoriartyApplication::SelectItemEventData(movie));
}

void HyperlinkHandler::handleTheatre(const char* link,  ulong_t length, const Point*)
{
    //MoriartyApplication& app = MoriartyApplication::instance();
    //MoviesMainForm* form = static_cast<MoviesMainForm*>(app.getOpenForm(moviesMainForm));
    //if (NULL == form || NULL == form->theatres)
    //{
    //    assert(false);
    //    return;
    //}
    //const char* data = hyperlinkData(link, length);
    //int theatre = -1;
    //const UniversalDataFormat& theatres = *form->theatres;
    //const uint_t theatresCount = theatres.getItemsCount()/2;
    //for (uint_t i = 0; i< theatresCount; ++i)
    //    if (StrEquals(theatres.getItemText(2*i, theatreNameIndex), data))
    //        theatre = i;
    //assert(-1 != theatre);
    //sendEvent(MoriartyApplication::appSelectTheatreEvent, MoriartyApplication::SelectItemEventData(theatre));
}

void HyperlinkHandler::handleMenu(const char* link,  ulong_t length, const Point* point)
{
    //const char* data = hyperlinkData(link, length);
    //if (NULL != data)
    //    sendTextNEvent(MoriartyApplication::extEventShowMenu, data, length, point);
}

void HyperlinkHandler::handleAmazonPreferences(const char* link,  ulong_t length, const Point*)
{
    //const char* data = hyperlinkData(link, length);
    //MoriartyApplication& app = MoriartyApplication::instance();
    //AmazonMainForm* form = static_cast<AmazonMainForm*>(app.getOpenForm(amazonMainForm));
    //if (StrEquals("showall", data))
    //{
    //    app.preferences().amazonPreferences.smallMain = false;
    //    form->levelMain();
    //    return;
    //}

    //if (StrEquals("showmajor", data))
    //{
    //    app.preferences().amazonPreferences.smallMain = true;
    //    form->levelMain();
    //    return;
    //}
    //assert(false);
}

void HyperlinkHandler::handleAmazonForm(const char* link,  ulong_t length, const Point*)
{
    //const char* data = hyperlinkData(link, length);
    //MoriartyApplication& app = MoriartyApplication::instance();
    //AmazonMainForm* form = static_cast<AmazonMainForm*>(app.getOpenForm(amazonMainForm));

    //if (StrEquals("up", data))
    //{
    //    form->levelUp();
    //    return;
    //}

    //if (StrEquals("main", data))
    //{
    //    form->levelMain();
    //    return;
    //}

    //if (StrEquals("search", data))
    //{
    //    form->handleSearch();
    //    return;
    //}

    //assert(false);
}

void HyperlinkHandler::handleListsOfBestsForm(const char* link,  ulong_t length, const Point*)
{
    //const char* data = hyperlinkData(link, length);
    //MoriartyApplication& app = MoriartyApplication::instance();
    //ListsOfBestsMainForm* form = static_cast<ListsOfBestsMainForm*>(app.getOpenForm(listsOfBestsMainForm));
    //if (StrEquals("search", data))
    //{
    //    form->handleSearch();
    //    return;
    //}

    //if (StrEquals("main", data))
    //{
    //    form->showMain();
    //    return;
    //}

    //assert(false);
}

void HyperlinkHandler::handleNetflixForm(const char* link,  ulong_t length, const Point*)
{
    //const char* data = hyperlinkData(link, length);
    //MoriartyApplication& app = MoriartyApplication::instance();
    //NetflixMainForm* form = static_cast<NetflixMainForm*>(app.getOpenForm(netflixMainForm));
    //if (StrEquals("main", data))
    //{
    //    form->showStartPage();
    //    return;
    //}

    //if (StrEquals("search", data))
    //{
    //    form->handleSearch();
    //    return;
    //}

    //if (StrEquals("login", data))
    //{
    //    form->handleLogin();
    //    return;
    //}

    //if (StrEquals("logout", data))
    //{
    //    form->handleLogout();
    //    return;
    //}

    //if (startsWith(data,"mov;"))
    //{
    //    form->handleMoveOnPosition(&data[4]);
    //    return;
    //}

    //if (StrEquals("queue", data))
    //{
    //    form->handleQueue();
    //    return;
    //}

    //assert(false);
}

void HyperlinkHandler::handleSimpleFormWithDefinition(const char* link, ulong_t length, const Point*)
{
    //const char* data = hyperlinkData(link, length);
    //MoriartyApplication& app = MoriartyApplication::instance();
    //long pos = StrFind(data, length, ';');
    //assert(-1 != pos);
    //DynStr* ds = DynStrNew(35);
    //ds = DynStrAppendData(ds, data, pos);
    //(new SimpleTextDoneForm(app, DynStrGetCStr(ds), &data[pos + 1]))->popup();
    //DynStrDelete(ds);
}

void HyperlinkHandler::handlePedia(const char* link, ulong_t length, const Point* point)
{
    const char* data = hyperlinkData(link, length);
    if (StrEquals(data, length, pediaUrlPartHome))
        handlePediaHome(data, length, point);
    else if (StrEquals(data, length, pediaUrlPartSearchDialog))
        handlePediaSearch(data, length, point);
    else if (StrEquals(data, length, pediaUrlPartShowArticle))
        handlePediaArticle(data, length, point);
    else if (StrStartsWith(data, length, pediaUrlPartSetLang, -1))
        handlePediaLang(data, length, point);
}


void HyperlinkHandler::handlePediaHome(const char* link, ulong_t length, const Point* point)
{
    //MoriartyApplication& app = MoriartyApplication::instance();
    //PediaMainForm* form = (PediaMainForm*)app.getOpenForm(pediaMainForm);
    //if (NULL == form)
    //    return;
    //form->handleAbout();
}

void HyperlinkHandler::handlePediaArticle(const char* link, ulong_t length, const Point* point)
{
    //MoriartyApplication& app = MoriartyApplication::instance();
    //PediaMainForm* form = (PediaMainForm*)app.getOpenForm(pediaMainForm);
    //if (NULL == form)
    //    return;
    //form->handleArticle();
}

void HyperlinkHandler::handlePediaLang(const char* link, ulong_t length, const Point* point)
{
    //const char* data = hyperlinkData(link, length);
    //MoriartyApplication& app = MoriartyApplication::instance();
    //PediaMainForm* form = (PediaMainForm*)app.getOpenForm(pediaMainForm);
    //if (NULL == form)
    //    return;
    //form->changeLanguage(data);
}

void HyperlinkHandler::handlePediaSearch(const char* link, ulong_t length, const Point* point)
{
    //MoriartyApplication& app = MoriartyApplication::instance();
    //PediaMainForm* form = (PediaMainForm*)app.getOpenForm(pediaMainForm);
    //if (NULL == form)
    //    return;
    //form->handleSearch();
}

void HyperlinkHandler::handleClipboardCopy(const char* link, ulong_t length, const Point* point)
{
    //const char* data = hyperlinkData(link, length);
    //ClipboardAddItem(clipboardText, data, length);
}

void HyperlinkHandler::handleRunModule(const char* link,  ulong_t length, const Point*)
{
    //const char* data = hyperlinkData(link, length);
    //MoriartyApplication& app = MoriartyApplication::instance();
    //MoriartyModule* module = app.getModuleByName(data);
    //if (NULL != module)
    //{
    //    app.runModuleById(module->id);    
    //}
}

//static bool ExtractEBookParams(const char* data, ulong_t length, const char* prefix, UInt16& srcVol, CDynStr& name, UInt16& trgVol)
//{
//    long pos = tstrlen(prefix) + 1;
//    if (pos >= length)
//        return false;
//        
//    data += pos;
//    length -= pos;
//    
//    bool target = true;
//    pos = StrFind(data, length, ';');
//    if (-1 == pos)
//    {
//        target = false;
//        pos = length;
//    }
//    
//    long p = StrFind(data, pos, ':');
//    if (-1 == p)
//        return false;
//    
//    long val;
//    Err err = numericValue(data, data + p, val, 16);
//    if (errNone != err)
//        return false;
//    
//    srcVol = val;
//    if (NULL == name.AppendCharPBuf(data + (p + 1), pos - (p + 1)))
//        return false;
//    
//    if (!target)
//        return true;
//    
//    err = numericValue(data + (pos + 1), data + length, val, 16);
//    if (errNone != err)
//        return false;
//        
//    trgVol = val;
//    return true;
//}

void HyperlinkHandler::handleEBook(const char* hyperlink, ulong_t length, const Point* point)
{
//    const char* data = hyperlinkData(hyperlink, length);
//    MoriartyApplication& app = MoriartyApplication::instance();
//    EBookPreferences& prefs = app.preferences().ebookPrefs;
//    EBookMainForm* form = (EBookMainForm*)app.getOpenForm(ebookMainForm);
//
//    UInt16 srcVol;
//    CDynStr str;
//    UInt16 trgVol;
//    Err err = errNone;
//    
//    if (StrStartsWith(data, length, ebookUrlPartBrowse, -1))
//    {
//        data = hyperlinkData(data, length);
//        if (NULL == str.AppendCharP2(urlSchemaEBookBrowse urlSeparatorSchemaStr, data))
//            goto NoMem;
//        
//        if (0 != length && NULL == str.AppendCharP("; "))
//            goto NoMem;
//            
//        if (NULL == str.AppendCharP(prefs.requestedFormats))
//            goto NoMem;
//        
//        handleHyperlink(str.GetCStr(), str.Len(), point);
//    }
//    else if (StrEquals(data, length, ebookUrlPartSearch))
//    {
//        if (NULL == form)
//            return;
//        form->handleSearch();
//    }
//    else if (StrStartsWith(data, length, ebookUrlPartSearch, -1))
//    {
//        data = hyperlinkData(data, length);
//        
//        if (NULL == str.AppendCharP3(urlSchemaEBookSearch urlSeparatorSchemaStr, data, "; "))
//            goto NoMem;
//        if (NULL == str.AppendCharP(prefs.requestedFormats))
//            goto NoMem;
//        
//        handleHyperlink(str.GetCStr(), str.Len(), point);
//    }
//    else if (StrStartsWith(data, length, ebookUrlPartDownload, -1))
//    {
//        data = hyperlinkData(data, length);
//        handleEBookDownload(data, length);
//    }
//    else if (StrStartsWith(data, length, ebookUrlPartCopy, -1))
//    {
//        if (NULL == form)
//            return;
//            
//        if (ExtractEBookParams(data, length, ebookUrlPartCopy, srcVol, str, trgVol))
//            err = eBook_Copy(srcVol, str.GetCStr(), trgVol);
//        form->refreshDownloadedEBooks();
//    }
//    else if (StrStartsWith(data, length, ebookUrlPartDelete, -1))
//    {
//        if (NULL == form)
//            return;
//
//        if (ExtractEBookParams(data, length, ebookUrlPartDelete, srcVol, str, trgVol))
//            err = eBook_Delete(srcVol, str.GetCStr());
//        form->refreshDownloadedEBooks();
//    }
//    else if (StrStartsWith(data, length, ebookUrlPartMove, -1))
//    {
//        if (NULL == form)
//            return;
//
//        if (ExtractEBookParams(data, length, ebookUrlPartMove, srcVol, str, trgVol))
//            err = eBook_Move(srcVol, str.GetCStr(), trgVol);
//        form->refreshDownloadedEBooks();
//    }
//    else if (StrStartsWith(data, length, ebookUrlPartLaunch, -1))
//    {
//        if (ExtractEBookParams(data, length, ebookUrlPartLaunch, srcVol, str, trgVol))
//        {
//            EBookFormat format;
//            err = eBook_DetectDatabaseFormat(srcVol, str.GetCStr(), format);
//            if (errNone != err)
//                goto Done;    
//            EBookReader reader = eBook_DetectReaderForFormat(format);
//            if (ebookReader_None != reader)
//                err = eBook_LaunchReader(srcVol, str.GetCStr(), reader);
//        }
//    }
//    else if (StrEquals(data, length, ebookUrlPartManage, -1))
//    {
//        if (NULL == form)
//            return;
//        
//        form->manage();        
//    }
//    else
//        assert(false);
//    return;
//NoMem:
//    err = memErrNotEnoughSpace;    
//Done:        
//    if (memErrNotEnoughSpace == err)
//        app.alert(notEnoughMemoryAlert);
//    else if (errNone != err)
//        app.alert(unableToCompeleteOperationAlert);
//    return;
}
    
void HyperlinkHandler::handleEBayForm(const char* link,  ulong_t length, const Point*)
{
    //const char* data = hyperlinkData(link, length);
    //MoriartyApplication& app = MoriartyApplication::instance();
    //EBayMainForm* form = static_cast<EBayMainForm*>(app.getOpenForm(eBayMainForm));
    //if (StrEquals("main", data))
    //{
    //    form->showStartPage();
    //    return;
    //}

    //if (StrEquals("search", data))
    //{
    //    form->handleSearch();
    //    return;
    //}

    //if (StrEquals("login", data))
    //{
    //    form->handleLogin();
    //    return;
    //}

    //if (StrEquals("logout", data))
    //{
    //    form->handleLogout();
    //    return;
    //}
    //
    //if (startsWith(data,"bid;"))
    //{
    //    form->handleBid(&data[4]);
    //    return;
    //}

    //if (startsWith(data,"resend;"))
    //{
    //    Preferences::EBayPreferences* prefs = &app.preferences().eBayPreferences;
    //    assert(NULL != prefs->encryptedCredentials);
    //    if (NULL != prefs->encryptedCredentials)
    //    {
    //        DynStr* str = DynStrFromCharP2(&data[7], prefs->encryptedCredentials);
    //        if (NULL != str)
    //        {
    //            app.lookupManager->fetchUrl(DynStrGetCStr(str));
    //            DynStrDelete(str);
    //        }        
    //    }
    //    return;
    //}

    //assert(false);
}

void HyperlinkHandler::handleFlickr(const  char* hyperlink, ulong_t len, const Point* point)
{
    //const char* data = hyperlinkData(hyperlink, len);
    //MoriartyApplication& app = MoriartyApplication::instance();
    //if (StrEquals(flickrUrlPartAbout, data, len))
    //{
    //    FlickrAboutForm* form = new_nt FlickrAboutForm(app);
    //    if (NULL == form)
    //    {
    //        app.alert(notEnoughMemoryAlert);
    //        return;
    //    }
    //    app.popupForm(form);
    //    return;
    //}
    //assert(false);
}

void HyperlinkHandler::handleEBookDownload(const char* data, ulong_t len)
{
//    if ((len % 2) != 0)
//        return;
//
//    char* d = StrUnhexlify(data, len);
//    if (NULL == d)
//        return;
//        
//    char* dd = d;
//    
//    long pos = StrFind(d, -1, _T(';'));
//    if (-1 == pos)
//        goto Finish;
//    
//    d += (pos + 1);
//    pos = StrFind(d, -1, _T(';'));
//    if (-1 == pos)
//        goto Finish;
//            
//    d += (pos + 1);
//    pos = StrFind(d, -1 , _T(':'));
//    if (-1 == pos)
//        goto Finish;
//        
//    // strip(d, *((ulong_t*)&pos));
//    
//    EBookFormats fs = eBook_DetectAvailableFormats();
//    EBookFormat f = eBook_ParseFormatName(d, pos);
//    if (ebookFormat_Unknown == f)
//        goto Finish;
//        
//    bool request = false;
//    MoriartyApplication& app = MoriartyApplication::instance();
//    EBookPreferences& prefs = app.preferences().ebookPrefs;
//    if (0 == (fs & f) && !prefs.dontConfirmDownloadWithNoReader)
//    {
//        EBookNoReaderForm* form = new_nt EBookNoReaderForm(MoriartyApplication::instance());
//        if (NULL == form)
//            goto Finish;
//        Err err = form->initialize();
//        if (errNone != err)
//            goto FinishForm;
//            
//        if (downloadButton == form->showModal())
//            request = true;
//FinishForm:            
//        delete form;
//    }
//    else
//        request = true;
//        
//    if (request)
//    {
//        CDynStr str;
//        if (NULL == str.AppendCharP2(urlSchemaEBookDownload urlSeparatorSchemaStr, data))
//            goto Finish;
//        handleHyperlink(str.GetCStr(), str.Len(), NULL);
//    }
//
//Finish:
//    if (NULL != dd)
//        free(dd);   
}
