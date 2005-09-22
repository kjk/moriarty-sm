#include "CurrencyModule.h"

#ifdef _WIN32
#include "CurrencyMainDialog.h"
#endif

MODULE_STARTER_DEFINE(Currency);

CurrencyPrefs::CurrencyPrefs():
    commonCurrenciesCount_(0)
{
}

CurrencyPrefs::~CurrencyPrefs()
{
}

void CurrencyPrefs::serialize(Serializer& ser)
{
    uint_t count = selectedCurrencies.size();
    uint_t index;
    ser(commonCurrenciesCount_);
    ser(count);
    if (Serializer::directionInput == ser.direction())
    {
        selectedCurrencies.clear();
        selectedCurrencies.reserve(count);
        for (uint_t i = 0; i< count; ++i)
        {
            ser(index);
            selectedCurrencies.push_back(index);
        }
    }
    else 
    {
        for (uint_t i = 0; i < count; ++i)
        {
            index = selectedCurrencies[i];
            ser(index);
        }
    }
}

//status_t selectCurrency(uint_t index);
//void deselectCurrency(uint_t index);
//bool isCurrencySelected(uint_t index);
