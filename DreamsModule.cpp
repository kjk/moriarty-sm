#include "DreamsModule.h"
#include "MoriartyStyles.hpp"
#include "HyperlinkHandler.h"
#include "LookupManager.h"
#include "InfoManPreferences.h"

#ifdef _WIN32
#include "DreamsMainDialog.h"
#endif

#include <Text.hpp>
#include <Definition.hpp>
#include <UniversalDataHandler.hpp>
#include <TextElement.hpp>
#include <LineBreakElement.hpp>
#include <UTF8_Processor.hpp>

MODULE_STARTER_DEFINE(Dreams)

DreamsPrefs::DreamsPrefs():
    downloadedTerm(NULL),
    pendingTerm(NULL)
{
}

DreamsPrefs::~DreamsPrefs()
{
    free(downloadedTerm);
    free(pendingTerm); 
} 

void DreamsPrefs::serialize(Serializer& ser)
{
    ser.text(downloadedTerm);
}

enum {
    dreamTitleIndex,
    dreamTextIndex
};

#define TXT(text) if (errNone != model->appendText(text)) goto Error
#define LBR()    if (errNone != model->append(new_nt LineBreakElement())) goto Error

DefinitionModel* DreamExtractFromUDF(const UniversalDataFormat& udf, char_t*& title)
{
    DefinitionModel* model = new_nt DefinitionModel();
    if (NULL == model)
        return NULL;
       
    ulong_t count = udf.getItemsCount();
    for (ulong_t i = 0; i < count; ++i)
    {
        if (2 == udf.getItemElementsCount(i))
        {
            TXT(udf.getItemText(i, dreamTitleIndex));
            model->last()->setStyle(StyleGetStaticStyle(styleNameHeader));
            LBR();
    
            if (errNone != DefinitionParseSimple(*model, udf.getItemData(i, dreamTextIndex), -1, true, urlSchemaDream))
                goto Error;

            LBR();
            LBR();
        }
        else if (1 == udf.getItemElementsCount(i))
        {
            free(title);
            title = StringCopy(udf.getItemText(i, dreamTitleIndex));
            if (NULL == title)
                goto Error;

            TXT(udf.getItemText(i, dreamTitleIndex));
            model->last()->setStyle(StyleGetStaticStyle(styleNamePageTitle));
            model->last()->setJustification(DefinitionElement::justifyCenter);
            LBR();
        }
        else
            goto Error;
    }    
    return model;
Error:
    delete model;
    return NULL;   
}

status_t DreamsDataRead(DefinitionModel*& m, char_t*& title)
{
    UniversalDataFormat* udf = new_nt UniversalDataFormat();
    if (NULL == udf)
        return memErrNotEnoughSpace;
        
    status_t err = UDF_ReadFromStream(dreamsDataStream, *udf);
    if (errNone == err) 
    { 
        DefinitionModel* model = DreamExtractFromUDF(*udf, title);
        if (NULL != model)
        {
            delete m;
            m = model;
        }
    }
   
    delete udf;
    return errNone;     
}

void HyperlinkHandler::handleDream(const char* link,  ulong_t length, const Point*)
{
    const char* data = hyperlinkData(link, length);
    LookupManager* lm = GetLookupManager();
    DreamsPrefs& prefs = GetPreferences()->dreamsPrefs;
    char_t* term = UTF8_ToNative(data, length);
    if (NULL == term)
        return;

    free(prefs.pendingTerm);
    prefs.pendingTerm = term;
         
    char* url = StringCopy(urlSchemaGetDream urlSeparatorSchemaStr);
    if (NULL != url && NULL != (url = StrAppend(url, -1, data, length)))
    { 
        lm->fetchUrl(url);
        free(url);
    }
}

