#ifndef STOCKS_MODULE_H__
#define STOCKS_MODULE_H__

#include "Modules.h"

#include <Utility.hpp>
#include <Serializer.hpp>
#include <vector>

MODULE_STARTER_DECLARE(Stocks);

class StocksPortfolio: public Serializable {

    struct Entry: private NonCopyable {
        char_t* symbol;
        ulong_t quantity;
        
        ~Entry();
        Entry();
    }; 
    
    typedef std::vector<Entry*> Entries_t;
    Entries_t entries_;
    
    char_t* name_;
    char_t* value_;
   
public:
    
    StocksPortfolio();
    ~StocksPortfolio();
   
    status_t initialize(const char_t* name);     
    
    void serialize(Serializer& ser);

    const char_t* name() const {return name_;}
    bool addSymbol(const char_t* symbol, ulong_t quantity);
    ulong_t size() const {return entries_.size();}
    const char_t* symbol(ulong_t index) const {return entries_[index]->symbol;}
    ulong_t quantity(ulong_t index) const {return entries_[index]->quantity;}
    void setQuantity(ulong_t index, ulong_t quantity) {entries_[index]->quantity = quantity;}
    void removeSymbol(ulong_t index); 
      
};

class StocksPrefs: public Serializable {
    
    typedef std::vector<StocksPortfolio*> Portfolios_t;
    Portfolios_t portfolios_;
   
public:

    enum {portfolioIndexInvalid = -1};
    int currentPortfolio;
    int downloadedPortfolio; 

    StocksPrefs();
    ~StocksPrefs(); 

    StocksPortfolio* addPortfolio(const char_t* name);
    ulong_t portfolioCount() const {return portfolios_.size();}
    void removePortfolio(ulong_t index);
    StocksPortfolio* portfolio(ulong_t index) const {return portfolios_[index];} 
   
    void serialize(Serializer& ser); 
     
};
       
#endif // STOCKS_MODULE_H__