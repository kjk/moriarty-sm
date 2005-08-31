#include "HoroscopesModule.h"
#include "HyperlinkHandler.h"
#include "LookupManager.h"

#ifdef _WIN32
#include "HoroscopesMainDialog.h"
#endif

#include <Text.hpp>
#include <UniversalDataHandler.hpp>

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

DefinitionModel* HoroscopeExtractFromUDF(const UniversalDataFormat& udf)
{
    DefinitionModel* model = new_nt DefinitionModel();
    if (NULL == model)
        return NULL;
    
    ulong_t size = udf.getItemsCount();       
    for (ulong_t i=0; i < size; i++)
    {
        if (2 != udf.getItemElementsCount(i))
            goto Error;
            
        const char* type = udf.getItemData(i, 0);
        if (StrEquals(horoscopeTitle, type))
        {
            //elems.push_back(text=new TextElement(item.getItemText(i,1)));
            //text->setStyle(StyleGetStaticStyle(styleNameBold));
            //text->setJustification(DefinitionElement::justifyCenter);
        }
        else if (StrEquals(horoscopeText, type))
        {
            //elems.push_back(new LineBreakElement());
            //elems.push_back(text=new TextElement(item.getItemText(i,1)));
        }
        else if (StrEquals(horoscopeSection, type))
        {
            //elems.push_back(new LineBreakElement());
            //elems.push_back(new LineBreakElement());
            //elems.push_back(text=new TextElement(item.getItemText(i,1)));
            //text->setStyle(StyleGetStaticStyle(styleNameBold));
        }
        else if (StrEquals(horoscopeSmallSection, type))
        {
            //elems.push_back(new LineBreakElement());
            //elems.push_back(text=new TextElement(item.getItemText(i,1)));
            //text->setStyle(StyleGetStaticStyle(styleNameBold));
        }
        else if (StrEquals(horoscopeDate, type))
        {
            //dateString_ = item.getItemText(i,1);
        }
        else if (StrEquals(horoscopeUrlLink, type))
        {
            //elems.push_back(new LineBreakElement());
            //elems.push_back(text=new TextElement(item.getItemText(i,1)));
            //String empty;
            //text->setHyperlink(empty, hyperlinkCallback);
            //text->setActionCallback(horoscopeLinkCallback, (void*) ((char_t*)item.getItemText(i,2)));
        }            
        else
            goto Error;
    }   
    return model;    
Error:
    delete model;
    return NULL;     
}
