#include "History.h"
#include "HyperlinkHandler.h"
#include "Modules.h"
#include "LookupManager.h"

#include <Text.hpp>
#include <HistoryCache.hpp>
#include <UniversalDataHandler.hpp>
#include <SysUtils.hpp>
#include <ByteFormatParser.hpp>
#include <ExtendedEvent.hpp>


// #include "PediaCacheDataRead.hpp"


static status_t UDF_CacheDataRead(const char_t* title, Reader& reader, LookupResult result)
{
    LookupManager* lm = GetLookupManager();
    delete lm->udf;
    lm->udf = new_nt UniversalDataFormat();
    if (NULL == lm->udf)
        return memErrNotEnoughSpace;
          
    status_t err= readUniversalDataFromReader(reader, *lm->udf);
    if (errNone != err)
        return err;

    LookupFinishedEventData* data = new_nt LookupFinishedEventData();
    if (NULL == data)
        return memErrNotEnoughSpace;

    data->result = result;
    ExtEventSendObject(extEventLookupFinished, data);
    return errNone;
}

static status_t BCF_CacheDataRead(const char_t* title, Reader& reader, LookupResult result)
{
    LookupManager* lm = GetLookupManager();
    delete lm->definitionModel;
    lm->definitionModel = NULL;  
    status_t err = ReadByteFormatFromReader(reader, lm->definitionModel);
    if (errNone != err)
        return err;
    
    lm->definitionModel->setTitle(title);
    LookupFinishedEventData* data = new_nt LookupFinishedEventData();
    if (NULL == data)
        return memErrNotEnoughSpace;
    
    data->result = result; 
    ExtEventSendObject(extEventLookupFinished, data);
    return errNone;
}

/*
static status_t AmazonCacheDataRead(const char_t* title, Reader& reader)
{
    return UDF_CacheDataRead(reader, lookupResultAmazon);
}

static status_t EBayCacheDataRead(const char_t* title, Reader& reader)
{
    return UDF_CacheDataRead(reader, lookupResultEBay);
}

static status_t NetflixCacheDataRead(const char_t* title, Reader& reader)
{
    return UDF_CacheDataRead(reader, lookupResultNetflix);
}

static status_t ListsOfBestsCacheDataRead(const char_t* title, Reader& reader)
{
    return UDF_CacheDataRead(reader, lookupResultListsOfBests);
}

static status_t LyricsCacheDataRead(const char_t* title, Reader& reader)
{
    return UDF_CacheDataRead(reader, lookupResultLyrics);
}

static status_t DictCacheDataRead(const char_t* title, Reader& reader)
{
    return UDF_CacheDataRead(reader, lookupResultDictDef);
}

static status_t EBookSearchDataRead(const char_t* title, Reader& reader)
{
    return BCF_CacheDataRead(reader, title, lookupResultEBookSearchResults);
}

static status_t EBookBrowseDataRead(const char_t* title, Reader& reader)
{
    return BCF_CacheDataRead(reader, title, lookupResultEBookBrowse);
}

static status_t EBookWelcomeDataRead(const char_t* title, Reader& reader)
{
    return BCF_CacheDataRead(reader, title, lookupResultEBookHome);
}
*/

typedef status_t (* CacheDataReadHandler)(const char_t* title, Reader&, LookupResult);

struct UrlSchemaToCacheNameEntry
{
    const char*        urlSchema;
    const char_t*        cacheName;
    CacheDataReadHandler readHandler;
    LookupResult         result; 
};

static UrlSchemaToCacheNameEntry schemaToCacheNameDispatch[] = 
{
/*
    {urlSchemaDictTerm,           dictHistoryCacheName,         DictCacheDataRead},
    {urlSchemaDictRandom,         dictHistoryCacheName,         DictCacheDataRead},
    
    {urlSchemaEncyclopediaTerm,   pediaHistoryCacheName,        PediaCacheDataRead},
    {urlSchemaEncyclopediaSearch, pediaHistoryCacheName,        PediaSearchDataRead},
 */
     
    {urlSchemaEBookBrowse,          ebookHistoryCacheName,          BCF_CacheDataRead, lookupResultEBookBrowse},
    {urlSchemaEBookHome,          ebookWelcomeCacheName,          BCF_CacheDataRead, lookupResultEBookHome},
    {urlSchemaEBookSearch,          ebookHistoryCacheName,          BCF_CacheDataRead, lookupResultEBookSearchResults},
    
 /*    
    {_T("s+amazonbrowse"),        amazonHistoryCacheName,       AmazonCacheDataRead},
    {_T("s+amazonsearch"),        amazonHistoryCacheName,       AmazonCacheDataRead},
    {_T("s+amazonitem"),          amazonHistoryCacheName,       AmazonCacheDataRead},
    {_T("s+amazonlist"),          amazonHistoryCacheName,       AmazonCacheDataRead},
    {_T("s+amazonwishlist"),      amazonHistoryCacheName,       AmazonCacheDataRead},
    
    {_T("s+netflixbrowse"),       netflixHistoryCacheName,      NetflixCacheDataRead},
    {_T("s+netflixsearch"),       netflixHistoryCacheName,      NetflixCacheDataRead},
    {_T("s+netflixitem"),         netflixHistoryCacheName,      NetflixCacheDataRead},
    
    {_T("s+listsofbestsitem"),    listsOfBestsHistoryCacheName, ListsOfBestsCacheDataRead},
    {_T("s+listsofbestssearch"),  listsOfBestsHistoryCacheName, ListsOfBestsCacheDataRead},
    {_T("s+listsofbestsbrowse"),  listsOfBestsHistoryCacheName, ListsOfBestsCacheDataRead},

    {_T("s+ebay"),                eBayHistoryCacheName,         EBayCacheDataRead},
    {_T("hs+ebay"),               eBayHistoryCacheName,         EBayCacheDataRead},
    {_T("Hs+ebay"),               eBayHistoryCacheName,         EBayCacheDataRead},

    {_T("s+lyricssearch"),        lyricsHistoryCacheName,       LyricsCacheDataRead},
    {_T("s+lyricsitem"),          lyricsHistoryCacheName,       LyricsCacheDataRead} 
   */
    
};

static const UrlSchemaToCacheNameEntry* CacheNameForUrl(const char* url)
{
    assert(NULL != url);
    long i = StrFind(url, -1, urlSeparatorSchema);
    if (-1 == i)
        return NULL;

    char* schema = StringCopyN(url, i);
    if (NULL == schema)
        return NULL;
  
    const UrlSchemaToCacheNameEntry* cache = NULL;
    for (i = 0; i < ARRAY_SIZE(schemaToCacheNameDispatch); ++i)
    {
        if (StrEquals(schema, schemaToCacheNameDispatch[i].urlSchema))
        {
            cache = &schemaToCacheNameDispatch[i];
            break;
        }
    }
    free(schema); 
    return cache;
}

bool ReadUrlFromCache(HistoryCache& cache, const char* url)
{
    const UrlSchemaToCacheNameEntry* cacheData = CacheNameForUrl(url);
    if (NULL == cacheData || NULL == cacheData->readHandler)
        return false;
    
    ulong_t index = cache.entryIndex(url);
    if (HistoryCache::entryNotFound == index)
        return false;
    
    char_t* path = GetStorePath(cacheData->cacheName);
    bool res = StrEquals(path, cache.dataStore->name());
    free(path);
      
    if (!res)
    {
        assert(cache.entryIsOnlyLink(index));
        return false;
    }     

    DataStoreReader* reader = cache.readerForEntry(index);
    if (NULL == reader)
        return false;

    status_t err = cacheData->readHandler(cache.entryTitle(index), *reader, cacheData->result);
    delete reader;
    return (errNone == err);
}

bool ReadUrlFromCache(const char* url)
{
    const UrlSchemaToCacheNameEntry* cacheData = CacheNameForUrl(url);
    if (NULL == cacheData || NULL == cacheData->readHandler)
        return false;
        
    assert(StrStartsWith(url, -1, cacheData->urlSchema, -1));

    char_t* path = GetStorePath(cacheData->cacheName);
    if (NULL == path)
        return false;
          
    HistoryCache cache;
    status_t err = cache.open(path);
    free(path);
    if (errNone != err)
        return false;

    assert(StrStartsWith(url, -1, cacheData->urlSchema, -1));

    ulong_t index = cache.entryIndex(url);
    if (HistoryCache::entryNotFound == index)
        return false;

    if (errNone != (err = cache.moveEntryToEnd(index)))
        return false;

    DataStoreReader* reader = cache.readerForEntry(index);
    if (NULL == reader)
        return false;

    err = cacheData->readHandler(cache.entryTitle(index), *reader, cacheData->result);
    delete reader;
    return (errNone == err);
}

status_t ReadByteFormatFromReader(Reader& reader, DefinitionModel*& model)
{
    ByteFormatParser* parser = new_nt ByteFormatParser();
    if (NULL == parser)
        return memErrNotEnoughSpace;
    
    status_t err = FeedHandlerFromReader(*parser, reader);
    if (errNone != err)
    {
        delete parser;
        return err;
    }

    model = parser->releaseModel();
    delete parser;
    if (NULL == model)
        return memErrNotEnoughSpace;

    return errNone;
}

