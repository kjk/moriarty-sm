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
    
    void serialize(Serializer& ser);
    
    status_t selectCurrency(uint_t index);
    void deselectCurrency(uint_t index);
    bool isCurrencySelected(uint_t index);
    
    UniversalDataFormat* udf;

private:
    uint_t commonCurrenciesCount_;
};

status_t CurrencyFetchData();

#endif // CURRENCY_MODULE_H__