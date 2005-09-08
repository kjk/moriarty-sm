#ifndef STOCKS_MODULE_H__
#define STOCKS_MODULE_H__

#include "Modules.h"

#include <Utility.hpp>
#include <Serializer.hpp>
#include <vector>

MODULE_STARTER_DECLARE(Stocks);

class StocksPortfolio: public Serializable {

public:

    static const double valueNotAvailable;

    struct Entry: private NonCopyable {
        char_t* symbol;
        ulong_t quantity;
        double change;
        double percentChange;
        double trade;

    private:
        friend class StocksPortfolio;             
        ~Entry();
        Entry();
    }; 
 
private:  
    
    typedef std::vector<Entry*> Entries_t;
    Entries_t entries_;
    
    char_t* name_;
   
    void serialize(Serializer& ser, ulong_t version); 
   
public:

    
    StocksPortfolio();
    ~StocksPortfolio();
   
    status_t initialize(const char_t* name);   
   
    ulong_t schemaVersion() const;
   
    bool serializeInFromVersion(Serializer& ser, ulong_t version);
    
    void serialize(Serializer& ser);

    const char_t* name() const {return name_;}
    bool addSymbol(const char_t* symbol, ulong_t quantity);
    ulong_t size() const {return entries_.size();}

/*    
    const char_t* symbol(ulong_t index) const {return entries_[index]->symbol;}
    ulong_t quantity(ulong_t index) const {return entries_[index]->quantity;}
    void setQuantity(ulong_t index, ulong_t quantity) {entries_[index]->quantity = quantity;}
 */
    Entry& entry(ulong_t i) {return *entries_[i];}
    const Entry& entry(ulong_t i) const {return *entries_[i];}   
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
    StocksPortfolio& portfolio(ulong_t index) const {return *portfolios_[index];} 
   
    void serialize(Serializer& ser); 
     
};
       
#endif // STOCKS_MODULE_H__