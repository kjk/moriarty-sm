#include "LookupManager.h"
#include "InfoManConnection.h"
#include "InfoManPreferences.h"
#include "HyperlinkHandler.h"
#include "History.h"
#include "Modules.h"

#include <Definition.hpp>
#include <UniversalDataFormat.hpp>
#include <SysUtils.hpp>
#include <Text.hpp>
#include <HistorySupport.hpp>
#include <HistoryCache.hpp>

#ifdef _WIN32
#include "ConnectionProgressDialog.h"
#endif

struct ServerErrorMapping {
	ServerError code;
	uint_t alertId;
};

#ifdef _WIN32
#include "resource.h"
#define DEF_SERVER_ERR(error, code, palmAlert, winAlert) {(error), (winAlert)},
#endif

#ifdef _PALM_OS
#define DEF_SERVER_ERR(error, code, palmAlert, winAlert) {(error), (palmAlert)},
#endif

static const ServerErrorMapping serverErrors[] =  {
#include "ServerErrors.hpp"
};

static LookupManager* lookupManager = NULL;

LookupManager* GetLookupManager()
{
	if (NULL == lookupManager)
		lookupManager = new_nt LookupManager();
	return lookupManager;
}

void LookupManagerDispose()
{
	delete lookupManager;
	lookupManager = NULL;
}

LookupManager::LookupManager():
	LookupManagerBase(extEventLookupStarted),
	definitionModel(NULL),
	udf(NULL),
	strings(NULL),
	stringsCount(0),
	
	regCodeDaysToExpire(regCodeDaysNotSet),
	clientVersionChecked(false),
	eBookVersion(-1)
{}

LookupManager::~LookupManager()
{
	delete definitionModel;
	delete udf;
	StrArrFree(strings, stringsCount);
}

void LookupManager::setDefinitionModel(DefinitionModel* model)
{
    Guard g(*this);
	delete definitionModel;
	definitionModel = model;
}

void LookupManager::setUDF(UniversalDataFormat* udff)
{
    Guard g(*this);
	delete udf;
	udf = udff;
}

void LookupManager::setStrings(char_t** ss, ulong_t sc)
{
    Guard g(*this);
	StrArrFree(strings, stringsCount);
	strings = ss;
	stringsCount = sc;
}

void LookupManager::handleServerError(ServerError serverError)
{
    assert(serverErrorNone != serverError);

    uint_t errorsCount = ARRAY_SIZE(serverErrors);
#ifdef _PALM_OS
    uint_t alertId = unknownServerErrorAlert;
#endif
#ifdef _WIN32
	uint_t alertId = IDS_ALERT_UNKNOWN_SERVER_ERROR;
#endif   
    for (uint_t i = 0; i < errorsCount; ++i)
    {
        if (serverErrors[i].code == serverError)
        {
            alertId = serverErrors[i].alertId;
            break;
        }
    }
	Alert(alertId);
}

struct ErrorMapping {
	status_t error;
	uint_t alertId;
};

#ifdef _WIN32
#define DEF_ERROR(code, palmAlert, winAlert) {(code), (winAlert)},
#endif
#ifdef _PALM_OS
#define _DEF_ERROR(code, palmAlert, winAlert) {(code), (palmAlert)},
#endif

// TODO: add more error codes
static const ErrorMapping errors[] = {
	DEF_ERROR(SocketConnection::errResponseMalformed, malformedResponseAlert, IDS_ALERT_MALFORMED_RESPONSE)
	DEF_ERROR(memErrNotEnoughSpace, notEnoughMemoryAlert, IDS_ALERT_NOT_ENOUGH_MEMORY)
	DEF_ERROR(netErrTimeout, connectionTimedOutAlert, IDS_ALERT_CONNECTION_TIMEOUT)
};

void LookupManager::handleConnectionError(status_t error)
{
	assert(errNone != error);

    uint_t errorsCount = ARRAY_SIZE(errors);
    uint_t alertId = uint_t(-1);
    for (uint_t i = 0; i < errorsCount; ++i)
    {
        if (errors[i].error == error)
        {
            alertId = errors[i].alertId;
            break;
        }
    }
	if (uint_t(-1) != alertId) 
		Alert(alertId);
}

status_t LookupManager::fetchUrl(const char* url)
{
    // remove all flags except 's' from url
    // for now only 'c' flag is used
    int offset = 0;
    while (urlFlagServer != url[offset])
    {
        offset++;
        assert('\0' != url[offset]);
    }   
    assert(urlSeparatorFlags == url[offset+1]);
    // read from cache
    if (offset > 0)
    {
        if (urlFlagHistory == url[offset-1] || urlFlagHistoryInCache == url[offset-1])
            offset--;
    }       
    if (ReadUrlFromCache(&url[offset]))
        return errNone;

    InfoManConnection* conn = createConnection();
    if (NULL == conn)
        return memErrNotEnoughSpace;
    
    status_t err = conn->setUrl(url);
    if (errNone != err)
    {
        delete conn;
        return err; 
    }
    return enqueueConnection(conn); 
}

InfoManConnection* LookupManager::createConnection()
{
    InfoManConnection* conn = new_nt InfoManConnection(*this);
    if (NULL == conn)
        return NULL;
    
    Preferences* prefs = GetPreferences(); 
    conn->serverAddress = prefs->serverAddress;    
    conn->setTransferTimeout(ticksPerSecond() * 30L);
    return conn;
}

status_t LookupManager::enqueueConnection(InfoManConnection* conn)
{
    assert(NULL != conn);

    status_t error = conn->enqueue();
    if (errNone != error)
    { 
        delete conn;
        return error;
    }
#ifdef _PALM_OS
    MoriartyApplication::popupForm(connectionProgressForm);
#endif

#ifdef _WIN32
    ConnectionProgressDialog::showModal(ExtEventGetWindow());
    // ConnectionProgressDialog::create(ExtEventGetWindow());
#endif
    
    return errNone; 
}

const LookupFinishedEventData* LookupFinishedData(Event& event)
{
    assert(extEventLookupFinished == ExtEventGetID(event));
    const LookupFinishedEventData* data = static_cast<const LookupFinishedEventData*>(ExtEventGetObject(event));
    return data;    
}


bool LookupManager::handleLookupFinishedInForm(Event& event)
{  
    const LookupFinishedEventData* data = LookupFinishedData(event);
    assert(data != NULL);
    switch (data->result)
    {
        case lookupResultError:
            handleConnectionError(data->error);
            return true;
    
        case lookupResultServerError:
            handleServerError(data->serverError);  
            return true;
        
        case lookupResultConnectionCancelledByUser:
            return true;
            
        case lookupResultLocationUnknown:
            ALERT(locationUnknownAlert, IDS_ALERT_LOCATION_UNKNOWN);
            return true;

        case lookupResultNoResults:
            ALERT(noResultsAlert, IDS_ALERT_NO_RESULTS);
            return true;
    }
    return false;  
}

DefinitionModel* LookupManager::releaseDefinitionModel()
{
    Guard g(*this);
    DefinitionModel* m = definitionModel;
    definitionModel = NULL;
    return m;
}

UniversalDataFormat* LookupManager::releaseUDF()
{
    Guard g(*this);
    UniversalDataFormat* u = udf;
    udf = NULL;
    return u;   
}

bool HandleCrossModuleLookup(Event& event, const char_t* cacheName, const char_t* moduleName)
{
    assert(extEventLookupFinished == ExtEventGetID(event));
    const LookupFinishedEventData* data = LookupFinishedData(event); 
    assert(NULL != data);
    
    int moduleId = -1;
    switch (data->result)
    {
        case lookupResultPediaArticle:
        case lookupResultPediaSearch:
        case lookupResultPediaStats:
            moduleId = moduleIdPedia;
            break;
            
        case lookupResultLyrics:
            moduleId = moduleIdLyrics;
            break;
        
        case lookupResultAmazon:
            moduleId = moduleIdAmazon;
            break;
            
        case lookupResultListsOfBests:
            moduleId = moduleIdListsOfBests;
            break;

        case lookupResultNetflix:
        case lookupResultNetflixLoginUnknown:
        case lookupResultNetflixRequestPassword:
        case lookupResultNetflixLoginOk:
            moduleId = moduleIdNetflix;
            break;

        case lookupResultEBay:
        case lookupResultEBayNoCache:
        case lookupResultEBayLoginUnknown:
        case lookupResultEBayRequestPassword:
        case lookupResultEBayLoginOk:
            moduleId = moduleIdEBay;
            break;

        case lookupResultDictDef:
            moduleId = moduleIdDict;
            break;
        
        case lookupResultEBookSearchResults:
        case lookupResultEBookDownload:
        case lookupResultEBookBrowse:
        case lookupResultEBookHome:
            moduleId = moduleIdEBooks;
            break;

    }
    if (-1 == moduleId)
        return false;
    
    LookupManager* lm = GetLookupManager();
    lm->crossModuleLookup = true;
    lm->historyCacheName = cacheName;
    lm->moduleName = moduleName;
    ModuleRun(ModuleID(moduleId));
    ExtEventRepost(event);
    return true;
}

// TODO: remember to pass cache name through GetStorePath()
void FinishCrossModuleLookup(HistorySupport& history, const char_t* moduleName)
{
    char_t* str = NULL;
    LookupManager* lm = GetLookupManager();
    if (!lm->crossModuleLookup)
        return;
        
    lm->crossModuleLookup = false;
    if (NULL == lm->historyCacheName)
    {   
        Log(eLogWarning, _T("FinishCrossModuleLookup(): lm.historyCacheName is NULL, won't write return link."), true);
        return;
    }
    char_t* lastCacheName = GetStorePath(lm->historyCacheName);
    lm->historyCacheName = NULL;
    if (NULL == lastCacheName)
    {
        Log(eLogWarning, _T("FinishCrossModuleLookup(): GetStorePath() returned NULL, not enough memory?"), true);
        return; 
    }  
    
    HistoryCache thisModuleCache;
    status_t err = thisModuleCache.open(history.cacheName());
    if (errNone != err)
    {
        LogStrUlong(eLogError, _T("FinishCrossModuleLookup(): unable to open thisModuleCache: "), err);
        free(lastCacheName);
        return;
    }
    
    HistoryCache prevModuleCache;
    err = prevModuleCache.open(lastCacheName);
    free(lastCacheName); 
    if (errNone != err)
    {
        LogStrUlong(eLogError, _T("FinishCrossModuleLookup(): unable to open prevModuleCache: "), err);
        return;
    }
    ulong_t count = thisModuleCache.entriesCount();
    if (0 == count)
    {
        Log(eLogError, _T("FinishCrossModuleLookup(): thisModuleCache is empty, can't write return link's url."), true);
        return;
    }
    
    const char* url = thisModuleCache.entryUrl(history.currentHistoryIndex);
    const char_t* title = thisModuleCache.entryTitle(history.currentHistoryIndex);
   
    if (NULL == (str = StrAppend(str, -1, moduleName, -1)))
    {
         Log(eLogError, _T("FinishCrossModuleLookup(): str is NULL 1."), true);
         return;
    }
   
    if (NULL == (str = StrAppend(str, -1, _T(": "), -1)))
    {
         Log(eLogError, _T("FinishCrossModuleLookup(): str is NULL 2."), true);
         return;
    }

    if (NULL == (str = StrAppend(str, -1, title, -1)))
    {
         Log(eLogError, _T("FinishCrossModuleLookup(): str is NULL 3."), true);
         return;
    }
   
    err = prevModuleCache.removeEntry(url);
    if (errNone != err)
        LogStrUlong(eLogWarning, _T("FinishCrossModuleLookup(): unable to remove old entry from prevModuleCache: "), err);

    err = prevModuleCache.appendLink(url, str);
    free(str);
    str = NULL; 
    if (errNone != err)
    {
        LogStrUlong(eLogError, _T("FinishCrossModuleLookup(): unable to append link to prevModuleCache: "), err);
        return;
    }
    
    count = prevModuleCache.entriesCount();
    assert(0 != count); // We just wrote an entry to that cache.
    if (1 == count)
    {
        Log(eLogWarning, _T("FinishCrossModuleLookup(): prevModuleCache was empty, not possible to write return link to thisModuleCache."), true);
        return;
    }
    
    url = prevModuleCache.entryUrl(count - 2);
    title = prevModuleCache.entryTitle(count - 2);
    assert(NULL != lm->moduleName);
    
    if (NULL == (str = StrAppend(str, -1, lm->moduleName, -1)))
    {
         Log(eLogError, _T("FinishCrossModuleLookup(): str is NULL 4."), true);
         return;
    }
   
    if (NULL == (str = StrAppend(str, -1, _T(": "), -1)))
    {
         Log(eLogError, _T("FinishCrossModuleLookup(): str is NULL 5."), true);
         return;
    }

    if (NULL == (str = StrAppend(str, -1, title, -1)))
    {
         Log(eLogError, _T("FinishCrossModuleLookup(): str is NULL 6."), true);
         return;
    }
    
    err = thisModuleCache.removeEntry(url);
    if (errNone != err)
        LogStrUlong(eLogWarning, _T("FinishCrossModuleLookup(): unable to remove old entry from thisModuleCache: "), err);
    
    count = thisModuleCache.entriesCount();
    if (0 == count)
        err = thisModuleCache.appendLink(url, str);
    else
        err = thisModuleCache.insertLink(count - 1, url, str);
    free(str);
        
    if (errNone != err)
    {
        Log(eLogError, _T("FinishCrossModuleLookup(): unable to insert return link to into thisModuleCache: "), err);
        return;
    }
    history.currentHistoryIndex = thisModuleCache.entriesCount() - 1;
 }
