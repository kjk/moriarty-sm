#ifndef CURRENCY_MODULE_H__
#define CURRENCY_MODULE_H__

#include "Modules.h"
#include "CurrencyUtils.h"
#include <Serializer.hpp>

MODULE_STARTER_DECLARE(Currency);

struct UniversalDataFormat;

struct CurrencyPrefs: public Serializable {
    SelectedCurrencies_t selectedCurrencies;
    
    CurrencyPrefs();
    ~CurrencyPrefs();
    
    ulong_t schemaVersion() const {return 2;}
    
    bool serializeInFromVersion(Serializer& ser, ulong_t ver);
    
    void serialize(Serializer& ser);
    
    status_t selectCurrency(uint_t index);
    void deselectCurrency(uint_t index);
    bool isCurrencySelected(uint_t index);
    
    long selectedCurrencyIndex;
    double amount;
    
    UniversalDataFormat* udf;

private:
    uint_t commonCurrenciesCount_;
    
    void serializeVersion1(Serializer& ser);
};

status_t CurrencyFetchData();

#endif // CURRENCY_MODULE_H__