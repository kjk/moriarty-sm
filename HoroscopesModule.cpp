#include "HoroscopesModule.h"
#include "HyperlinkHandler.h"
#include "LookupManager.h"

#include "MoriartyStyles.hpp"

#ifdef _WIN32
#include "HoroscopesMainDialog.h"
#endif

#include <Text.hpp>
#include <UniversalDataHandler.hpp>
#include <DefinitionElement.hpp>
#include <LineBreakElement.hpp>


MODULE_STARTER_DEFINE(Horoscopes)

HoroscopesPrefs::HoroscopesPrefs():
    lastQuery(StringCopy("")),
    lastSign(signNotSet)
{
}

HoroscopesPrefs::~HoroscopesPrefs()
{
    free(lastQuery); 
}

void HoroscopesPrefs::serialize(Serializer& ser)
{
    ser(lastSign);
    ser.narrow(lastQuery); 
}

static const char* signNames[] = {
    "Aries",
    "Taurus",
    "Gemini",
    "Cancer",
    "Leo",
    "Virgo",
    "Libra", 
    "Scorpio",
    "Sagittarius",
    "Capricorn",
    "Aquarius",
    "Pisces"
}; 

static status_t HoroscopeFetch(const char* name)
{
    char* url = StringCopy(urlSchemaHoroscope urlSeparatorSchemaStr);
    if (NULL == url || NULL == (url = StrAppend(url, -1, name, -1)))
        return memErrNotEnoughSpace;
        
    status_t err = GetLookupManager()->fetchUrl(url);
    free(url);
    return err;
}

status_t HoroscopeFetch(uint_t index)
{
    assert(index < horoscopesSignCount);
    const char* name = signNames[index];
    return HoroscopeFetch(name);
}

static void HoroscopesLinkCallback(void* data)
{
    HoroscopeFetch((const char*)data);
}

#define horoscopeTitle         ("T")
#define horoscopeSection       ("S")
#define horoscopeSmallSection  ("s")
#define horoscopeText          ("t")
#define horoscopeDate          ("D")
#define horoscopeUrlLink       ("L")

#define TXT(text) if (errNone != model->appendText(text)) goto Error
#define LBR()    if (errNone != model->append(new_nt LineBreakElement())) goto Error

DefinitionModel* HoroscopeExtractFromUDF(const UniversalDataFormat& udf, char_t*& date)
{
    DefinitionModel* model = new_nt DefinitionModel();
    if (NULL == model)
        return NULL;
    
    ulong_t size = udf.getItemsCount();       
    for (ulong_t i=0; i < size; i++)
    {
        const char* type = udf.getItemData(i, 0);
        if (StrEquals(horoscopeTitle, type))
        {
            TXT(udf.getItemText(i, 1));
            model->last()->setStyle(StyleGetStaticStyle(styleNameBold));
            model->last()->setJustification(DefinitionElement::justifyCenter);
        }
        else if (StrEquals(horoscopeText, type))
        {
            LBR();
            TXT(udf.getItemText(i, 1));
        }
        else if (StrEquals(horoscopeSection, type))
        {
            LBR();
            LBR();
            TXT(udf.getItemText(i, 1));
            model->last()->setStyle(StyleGetStaticStyle(styleNameBold));
        }
        else if (StrEquals(horoscopeSmallSection, type))
        {
            LBR();
            TXT(udf.getItemText(i, 1));
            model->last()->setStyle(StyleGetStaticStyle(styleNameBold));
        }
        else if (StrEquals(horoscopeDate, type))
        {
            char_t* d = StringCopy(udf.getItemText(i, 1));
            free(date);
            date = d;
            if (NULL == date)
                goto Error;
        }
        else if (StrEquals(horoscopeUrlLink, type))
        {
            LBR();
            TXT(udf.getItemText(i, 1));
            model->last()->setActionCallback(HoroscopesLinkCallback, (void*)udf.getItemData(i, 2));
            model->last()->setHyperlink("", hyperlinkCallback);
        }            
        else
            goto Error;
    }   
    return model;    
Error:
    delete model;
    return NULL;     
}
