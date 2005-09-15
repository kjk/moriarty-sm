#ifndef STOCKS_MODULE_H__
#define STOCKS_MODULE_H__

#include "Modules.h"

#include <Utility.hpp>
#include <Serializer.hpp>
#include <vector>

MODULE_STARTER_DECLARE(Stocks);

struct StocksEntry: private NonCopyable {
    char* url;
    char_t* symbol;
    char_t* data;
     
    ulong_t quantity;
    double change;
    double percentChange;
    double trade;
   
    enum Status {
        statusUnknown,
        statusReady,
        statusChanged,
        statusAmbiguous
    } status;  

private:
    friend class StocksPortfolio;             
    ~StocksEntry();
    StocksEntry();
}; 
 
class StocksPortfolio: public Serializable {

public:
    typedef StocksEntry Entry;

    static const double valueNotAvailable;

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
   
    void replaceName(char_t* name); 

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
    const StocksPortfolio& portfolio(ulong_t index) const {return *portfolios_[index];} 
    StocksPortfolio& portfolio(ulong_t index) {return *portfolios_[index];} 
    StocksPortfolio& current() {return portfolio(currentPortfolio);}
    const StocksPortfolio& current() const {return portfolio(currentPortfolio);} 
   
    void serialize(Serializer& ser); 
     
};

bool StocksResyncEntry(StocksPortfolio::Entry& e, ulong_t skipPortfolio);
status_t StocksUpdate(bool validate = false);
struct UniversalDataFormat;
bool StocksUpdateFromUDF(const UniversalDataFormat& udf);
class DefinitionModel;
DefinitionModel* StocksDetailsFromUDF(const UniversalDataFormat& udf);
status_t StocksFetchDetails(const char* url, bool direct);
char_t* StocksValidateTicker(const char_t* ticker);
char_t* StocksValidatePortfolioName(const char_t* portfolio);
       
#endif // STOCKS_MODULE_H__