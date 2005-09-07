#include "StocksModule.h"

#include <Text.hpp>

#ifdef _WIN32
#include "StocksMainDialog.h"
#endif

MODULE_STARTER_DEFINE(Stocks)


StocksPortfolio::Entry::Entry():
    symbol(NULL),
    quantity(0)
{
}

StocksPortfolio::Entry::~Entry()
{
    free(symbol); 
}  

StocksPortfolio::StocksPortfolio():
    name_(NULL),
    value_(NULL)
{
}

StocksPortfolio::~StocksPortfolio()
{
    ulong_t size = entries_.size();
    for (ulong_t i = 0; i < size; ++i)
        delete entries_[i];
    free(name_);
    free(value_);   
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

void StocksPortfolio::serialize(Serializer& ser)
{
    ser.text(name_);
    ser.text(value_);
   
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
        ser.text(entries_[i]->symbol);
        ser(entries_[i]->quantity);
    }  
}

StocksPrefs::StocksPrefs():
    currentPortfolio(portfolioIndexInvalid),
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