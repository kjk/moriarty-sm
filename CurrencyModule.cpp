#include "CurrencyModule.h"
#include "HyperlinkHandler.h"
#include "LookupManager.h"

#include <algorithm>
#include <UniversalDataFormat.hpp>
#include <Currencies.hpp>

#ifdef _WIN32
#include "CurrencyMainDialog.h"
#endif

MODULE_STARTER_DEFINE(Currency);

CurrencyPrefs::CurrencyPrefs():
    commonCurrenciesCount_(0),
    udf(NULL),
    selectedCurrencyIndex(-1),
    amount(1.0)
{
    uint_t count = CommonCurrenciesCount();
    ErrTry {
        selectedCurrencies.reserve(count);
    }
    ErrCatch (ex) {
        (void)ex;
        return;
    }
    
    for (uint_t i = 0; i < count; ++i)
    {
        int index = CurrencyIndex(GetCommonCurrencySymbol(i));
        assert(-1 != index);
        selectedCurrencies.push_back(index);
    }
    commonCurrenciesCount_ = count;

}

CurrencyPrefs::~CurrencyPrefs()
{
    delete udf;
}

void CurrencyPrefs::serializeVersion1(Serializer& ser)
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

bool CurrencyPrefs::serializeInFromVersion(Serializer &ser, ulong_t ver)
{
    if (1 != ver)
        return false;
    serializeVersion1(ser);
    return true;
}

void CurrencyPrefs::serialize(Serializer& ser)
{
    serializeVersion1(ser);
    ser(selectedCurrencyIndex);
    ser(amount);
}

status_t CurrencyPrefs::selectCurrency(uint_t index)
{
    int common = GetCommonCurrencyIndex(index);
    uint_t pos;
    if (-1 != common)
    {
        for (pos = 0; pos < commonCurrenciesCount_; ++pos)
        {
            int i = GetCommonCurrencyIndex(selectedCurrencies[pos]);
            assert(-1 != i);
            if (common <= i)
                break;
        }
        ++commonCurrenciesCount_;
    }
    else {
        for (pos = commonCurrenciesCount_; pos < selectedCurrencies.size(); ++pos)
        {
            if (index < selectedCurrencies[pos])
                break;
        }
    }
    ErrTry {
        selectedCurrencies.insert(selectedCurrencies.begin() + pos, index);
    }
    ErrCatch (ex) {
        return ex;
    } ErrEndCatch;
    return errNone;
}

// TODO: convert functions below to not use iterators
void CurrencyPrefs::deselectCurrency(uint_t index)
{
    bool common = IsCommonCurrency(index);
    SelectedCurrencies_t::iterator pos = std::find(selectedCurrencies.begin(), selectedCurrencies.end(), index);
    assert(selectedCurrencies.end() != pos);
    selectedCurrencies.erase(pos);
    if (common)
        --commonCurrenciesCount_;
}

bool CurrencyPrefs::isCurrencySelected(uint_t index)
{
    SelectedCurrencies_t::const_iterator pos = std::find(selectedCurrencies.begin(), selectedCurrencies.end(), index);
    if (selectedCurrencies.end() != pos)
        return true;
    return false;
}

status_t CurrencyFetchData()
{
    const char* url = urlSchemaCurrency urlSeparatorSchemaStr;
    LookupManager* lm = GetLookupManager();
    return lm->fetchUrl(url);
}