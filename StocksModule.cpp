#include "StocksModule.h"
#include "InfoManPreferences.h"
#include "HyperlinkHandler.h"
#include "LookupManager.h"

#include <Definition.hpp>
#include <Text.hpp>
#include <UTF8_Processor.hpp>
#include <UniversalDataFormat.hpp>

#ifdef _WIN32
#include "StocksMainDialog.h"
#endif

MODULE_STARTER_DEFINE(Stocks)

const double StocksPortfolio::valueNotAvailable = DBL_MAX;

StocksEntry::StocksEntry():
    url(NULL),
    changed(NULL), 
    symbol(NULL),
    quantity(0),
    change(StocksPortfolio::valueNotAvailable),
    percentChange(StocksPortfolio::valueNotAvailable),
    trade(StocksPortfolio::valueNotAvailable),
    status(statusUnknown) 
{
}

StocksEntry::~StocksEntry()
{
    free(symbol); 
    free(changed);
    free(url);  
}  

StocksPortfolio::StocksPortfolio():
    name_(NULL)
{
}

StocksPortfolio::~StocksPortfolio()
{
    ulong_t size = entries_.size();
    for (ulong_t i = 0; i < size; ++i)
        delete entries_[i];
    free(name_);
}  

status_t StocksPortfolio::initialize(const char_t* name)
{
    free(name_);
    name_ = StringCopy(name);
    if (NULL == name)
        return memErrNotEnoughSpace;

    if (!addSymbol(_T("^DJI"), 0)
        || !addSymbol(_T("^IXIC"), 0)
        || !addSymbol(_T("^GSPC"), 0))
        return memErrNotEnoughSpace;

    return errNone;    
}

void StocksPortfolio::removeSymbol(ulong_t index)
{
    assert(index < entries_.size());
    delete entries_[index];
    entries_.erase(entries_.begin() + index); 
}

bool StocksPortfolio::addSymbol(const char_t* symbol, ulong_t quantity)
{
    Entry* entry = new_nt Entry();
    if (NULL == entry)
        return false;

    status_t err = errNone;
    ErrTry {
        entries_.push_back(entry);
    }
    ErrCatch (ex) {
        err = ex;
    } ErrEndCatch;
    if (errNone != err)
    {
        delete entry;
        return false;
    }
    if (NULL == (entry->symbol = StringCopy(symbol)))
    {
        delete entry;
        entries_.pop_back();
        return false; 
    }   
    entry->quantity = quantity;
    return true;  
}

void StocksPortfolio::replaceName(char_t* name)
{
    free(name_);
    name_ = name; 
}

ulong_t StocksPortfolio::schemaVersion() const {return 2;}

void StocksPortfolio::serialize(Serializer& ser, ulong_t version)
{
    ser.text(name_);
    
    if (1 == version)
    {  
        char_t* value = NULL; 
        ser.text(value);
        free(value);
    }
   
    uint_t size = entries_.size();
    ser(size);
    
    for (uint_t i = size; i < entries_.size(); ++i)
        delete entries_[i];
    entries_.resize(size); 
    
    for (uint_t i = 0; i < size; ++i)
    {
        if (Serializer::directionInput ==  ser.direction())
        {  
            if (NULL == entries_[i] && NULL == (entries_[i] = new_nt Entry()))
            {
                entries_.resize(i);
                ErrThrow(memErrNotEnoughSpace);
            }
        }
        Entry& e = *entries_[i];
        ser.text(e.symbol);
        ser(e.quantity);
        if (version > 1)
        {
            ser(e.change);
            ser(e.percentChange);
            ser(e.trade);
            ser.narrow(e.url);
            ser.text(e.changed);
            uint_t s = e.status;
            ser(s);
            e.status = StocksEntry::Status(s);
        }
    }  
}

bool StocksPortfolio::serializeInFromVersion(Serializer& ser, ulong_t version)
{
    if (1 == version)
    {
        serialize(ser, version);
        return true; 
    }  
    assert(false);
    return false;  
}

void StocksPortfolio::serialize(Serializer& ser)
{
    serialize(ser, schemaVersion());
}

StocksPrefs::StocksPrefs():
    currentPortfolio(0),
    downloadedPortfolio(portfolioIndexInvalid) 
{
    addPortfolio(TEXT("Default"));
}

StocksPrefs::~StocksPrefs()
{
    ulong_t size = portfolios_.size();
    for (ulong_t i = 0; i < size; ++i)
        delete portfolios_[i]; 
}

StocksPortfolio* StocksPrefs::addPortfolio(const char_t* name)
{
    StocksPortfolio* p = new_nt StocksPortfolio();
    if (NULL == p)
        return NULL;
    
    if (errNone != p->initialize(name))
    {
        delete p;
        return NULL; 
    }     
   
    status_t err = errNone;
    ErrTry {
        portfolios_.push_back(p);
    } 
    ErrCatch (ex) {
        err = ex;
    } ErrEndCatch;
    if (errNone != err) 
    {
        delete p;
        return NULL; 
    }        
    return p; 
}

void StocksPrefs::removePortfolio(ulong_t index)
{
    assert(index < portfolios_.size());
    delete portfolios_[index];
    portfolios_.erase(portfolios_.begin() + index);  
}


void StocksPrefs::serialize(Serializer& ser)
{
    ser
    (currentPortfolio)
    (downloadedPortfolio);
          
    uint_t size = portfolios_.size();
    ser(size);
    
    for (uint_t i = size; i < portfolios_.size(); ++i)
        delete portfolios_[i];
    portfolios_.resize(size); 
    
    for (uint_t i = 0; i < size; ++i)
    { 
        if (Serializer::directionInput == ser.direction())
        {
            if (NULL == portfolios_[i] && NULL == (portfolios_[i] = new_nt StocksPortfolio()))
            {
                portfolios_.resize(i);
                ErrThrow(memErrNotEnoughSpace); 
            }
        }
        ser(*portfolios_[i]);
    }
}

bool StocksResyncEntry(StocksEntry& e, ulong_t skipPortfolio)
{
    const StocksPrefs& prefs = GetPreferences()->stocksPrefs;

    ulong_t pc = prefs.portfolioCount();
    for (ulong_t  j = 0; j < pc; ++j)
    {
        if (j == skipPortfolio)
            continue;
             
        const StocksPortfolio& pp = prefs.portfolio(j);
        ulong_t ss = pp.size();
        for (ulong_t k = 0; k < ss; ++k)
        {
            const StocksEntry& ee = pp.entry(k);
            if (StrEquals(e.symbol, ee.symbol))
            {
                e.change = ee.change;
                e.percentChange = ee.percentChange;
                e.trade = ee.trade;
                return true;
            }   
        }
    }
    return false;
}

status_t StocksUpdate()
{
    char* url = StringCopy(urlSchemaStocksList urlSeparatorSchemaStr);
    if (NULL == url)
        return memErrNotEnoughSpace;  

    const StocksPrefs& prefs = GetPreferences()->stocksPrefs;
    
    //ulong_t size = prefs.portfolioCount();     
    //for (ulong_t i = 0; i < size; ++i)
    //{
    //    const StocksPortfolio& p = prefs.portfolio(i);
        const StocksPortfolio& p = prefs.portfolio(prefs.currentPortfolio); 
        ulong_t  s = p.size();
        for (ulong_t j = 0; j < s; ++j)
        { 
            bool first = ((j == 0)); // && (i == 0));
            if (!first && NULL == (url = StrAppend(url, -1, "; ", -1)))
                return memErrNotEnoughSpace;
            
            char* symbol = UTF8_FromNative(p.entry(j).symbol);
            if (NULL == symbol)
            {
                free(url);
                return memErrNotEnoughSpace; 
            }
            if (NULL == (url = StrAppend(url, -1, symbol, -1)))
            {
                free(symbol);
                return memErrNotEnoughSpace;
            }
            free(symbol); 
        }       
    //}
    LookupManager* lm = GetLookupManager();
    status_t err = lm->fetchUrl(url);
    free(url);
    return err;   
}

enum {
    stocksListItemUrlIndex,
    stocksListItemSymbolIndex,
    stocksListItemTimeIndex,
    stocksListItemTradeIndex,
    stocksListItemChangeIconIndex,
    stocksListItemChangeIndex,
    stocksListItemPercentChangeIndex,
    stocksListItemVolumeIndex,
    stocksListElementsCount
};

enum {
    stocksListNotFoundElementsCount = 2
};

//static bool StocksSyncEntriesToUDF(const UniversalDataFormat& udf, ulong_t i)
//{
//    const char_t* symbol = udf.getItemText(i, stocksListItemSymbolIndex);
//    if (NULL == symbol)
//        return false;
//
//    StocksPrefs& prefs = GetPreferences()->stocksPrefs;
//    ulong_t sz = prefs.portfolioCount();
//    for (ulong_t j = 0; j < sz; ++j)
//    {
//        StocksPortfolio& p = prefs.portfolio(j);
//        ulong_t ss = p.size();
//        for (ulong_t k = 0; k < ss; ++k)
//        {
//            StocksEntry& e = p.entry(k);
//            if (equalsIgnoreCase(symbol, e.symbol))
//            {
//                const char* trade = udf.getItemData(i, stocksListItemTradeIndex);
//                const char* change = udf.getItemData(i, stocksListItemChangeIndex);
//                const char* percent = udf.getItemData(i, stocksListItemPercentChangeIndex);
//                if (errNone != numericValue(trade, -1, e.trade))
//                    return false;
//                if (errNone != numericValue(change, -1, e.change))
//                    return false;
//                ulong_t len = Len(percent);
//                if (0 == len || '%' != percent[len - 1])
//                    return false;
//                if (errNone != numericValue(percent, len - 1, e.percentChange))
//                    return false; 
//            }
//        }
//    } 
//    return true;    
//}

bool StocksUpdateFromUDF(const UniversalDataFormat& udf)
{
    StocksPrefs& prefs = GetPreferences()->stocksPrefs;
    StocksPortfolio& p = prefs.portfolio(prefs.currentPortfolio);
    ulong_t len = udf.getItemsCount();
    assert(len == p.size()); 
    for (ulong_t i = 0; i < len; ++i)
    {
        StocksEntry& e = p.entry(i);
        assert(udf.getItemElementsCount(i) >= 2);
        
        e.url = StringCopy(udf.getItemData(i, stocksListItemUrlIndex));
        if (NULL == e.url)
            return false;
            
        const char_t* symbol = udf.getItemText(i, stocksListItemSymbolIndex);

        if (equalsIgnoreCase(symbol, e.symbol))
        {
            assert(stocksListElementsCount == udf.getItemElementsCount(i));
            e.status = e.statusReady;
            const char* trade = udf.getItemData(i, stocksListItemTradeIndex);
            const char* change = udf.getItemData(i, stocksListItemChangeIndex);
            const char* percent = udf.getItemData(i, stocksListItemPercentChangeIndex);
            if (errNone != numericValue(trade, -1, e.trade))
                return false;
            if (errNone != numericValue(change, -1, e.change))
                return false;
            ulong_t len = Len(percent);
            if (0 == len || '%' != percent[len - 1])
                return false;
            if (errNone != numericValue(percent, len - 1, e.percentChange))
                return false; 
        } 
        else if (equalsIgnoreCase(symbol, _T("?")))
        {
            assert(stocksListNotFoundElementsCount == udf.getItemElementsCount(i));
            e.status = e.statusAmbiguous;
        }
        else
        {
            assert(stocksListNotFoundElementsCount == udf.getItemElementsCount(i));
            free(e.changed);
            e.changed = StringCopy(symbol);
            if (NULL == e.changed)
                return false;
            e.status = e.statusChanged; 
        }
    }  
    return true;
}

enum {
    stocksNameIndex,
    stocksLastTradeIndex,
    stocksTradeTimeIndex,
    stocksChangeIconIndex,
    stocksChangeIndex,
    stocksPrevCloseIndex,
    stocksOpenIndex,
    stocksBidIndex,
    stocksAskIndex,
    stocks1yTargetEstIndex,
    stocksDaysRangeIndex,
    stocks52wkRangeIndex,
    stocksVolumeIndex,
    stocksAvgVolIndex,
    stocksMarketCapIndex,
    stocksPEIndex,
    stocksEPSIndex,
    stocksDivYieldIndex,
    stocksStockElementsCount
};


DefinitionModel* StocksDetailsFromUDF(const UniversalDataFormat& udf)
{
    DefinitionModel* model = new_nt DefinitionModel();
    if (NULL == model)
        return NULL;
        
    if (1 != udf.getItemsCount())
        goto Error;

    ulong_t size = udf.getItemElementsCount(0);
    for (ulong_t i = 0; i < size; ++i)
    {
         
    }  
    return model;
Error:
    delete model;
    return NULL;   
}

status_t StocksFetchDetails(const char_t* symbol)
{
    char* s = UTF8_FromNative(symbol);
    if (NULL == s)
        return memErrNotEnoughSpace;

    char* url = StringCopy(urlSchemaStock urlSeparatorSchemaStr);
    if (NULL == url || NULL == (url = StrAppend(url, -1, s, -1)))
    {
        free(s);
        return memErrNotEnoughSpace; 
    }     
    free(s);
    LookupManager* lm = GetLookupManager();
    status_t err = lm->fetchUrl(url);
    free(url);
    return err;     
}
