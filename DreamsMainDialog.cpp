#include "DreamsMainDialog.h"
#include "LookupManager.h"
#include "InfoMan.h"
#include "HyperlinkHandler.h"
#include "DreamsModule.h"
#include "InfoManPreferences.h"

#include <SysUtils.hpp>
#include <UTF8_Processor.hpp>
#include <Text.hpp>
#include <UniversalDataFormat.hpp>

DreamsMainDialog::DreamsMainDialog():
    ModuleDialog(IDR_DONE_SEARCH_MENU),
    title_(NULL) 
{
}

DreamsMainDialog::~DreamsMainDialog()
{
    free(title_);
} 

MODULE_DIALOG_CREATE_IMPLEMENT(DreamsMainDialog, IDD_DREAMS_MAIN)

bool DreamsMainDialog::handleInitDialog(HWND fw, long ip)
{
    createSipPrefControl();
    info_.attachControl(handle(), IDC_DREAMS_INFO);
    term_.attachControl(handle(), IDC_SEARCH_TERM);
    
	Rect r;
	innerBounds(r);
   	long h = term_.height() + LogY(2);
	renderer_.create(WS_TABSTOP, LogX(1), h, r.width() - 2 * LogX(1), r.height() - h - LogY(1), handle());
    renderer_.definition.setInteractionBehavior(
        Definition::behavHyperlinkNavigation
      | Definition::behavUpDownScroll
      //| Definition::behavMenuBarCopyButton
      | Definition::behavMouseSelection
    );  
    renderer_.definition.setHyperlinkHandler(GetHyperlinkHandler()); 

    ModuleDialog::handleInitDialog(fw, ip);
   
    overrideBackKey();
    
    DreamsPrefs& prefs = GetPreferences()->dreamsPrefs;
    DefinitionModel* model = NULL;
    DreamsDataRead(model, title_);
    if (NULL != model)
    {
        renderer_.setModel(model, Definition::ownModel);
        term_.setCaption(prefs.downloadedTerm);
        
        info_.hide();
        renderer_.show();
        renderer_.focus();
    }    
    else
        term_.focus();
    return false;
}

long DreamsMainDialog::handleResize(UINT st, ushort w, ushort height)
{
    term_.anchor(anchorRight, 2 * LogX(1), anchorNone, 0, repaintWidget);
    long h = term_.height() + LogY(2); 
    renderer_.anchor(anchorRight, 2 * LogX(1), anchorBottom, h + LogY(1), repaintWidget);
    info_.anchor(anchorRight, 2 * LogX(1), anchorBottom, h + LogY(1), repaintWidget);

    return ModuleDialog::handleResize(st, w, height);
}

long DreamsMainDialog::handleCommand(ushort nc, ushort id, HWND sender)
{
    switch (id) 
    {
        case ID_SEARCH:
        {
            ulong_t len;
            char_t* term = term_.caption(&len);
            if (NULL == term)
            {
                Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
                return messageHandled;
            }  
            const char_t* s = term;
            strip(s, len);
            if (0 != len)
                search(s, len);
            free(term);
            return messageHandled;
        }
    }
    return ModuleDialog::handleCommand(nc, id, sender);
}

bool DreamsMainDialog::handleLookupFinished(Event& event, const LookupFinishedEventData* data)
{
    LookupManager* lm = GetLookupManager();
    DreamsPrefs& prefs = GetPreferences()->dreamsPrefs; 
    switch (data->result)
    {
        case lookupResultDreamData:
        {
            UniversalDataFormat* udf = NULL;
            PassOwnership(lm->udf, udf);
            assert(NULL != udf);
            DefinitionModel* model = DreamExtractFromUDF(*udf, title_);
            delete udf;
            if (NULL == model)
                Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
            else
            { 
                info_.hide();
                renderer_.setModel(model, Definition::ownModel);
                renderer_.show();
                renderer_.focus();
            }
            free(prefs.downloadedTerm);
            prefs.downloadedTerm = prefs.pendingTerm;
            prefs.pendingTerm = NULL;
            return true;
        }
    }  
    free(prefs.pendingTerm);
    prefs.pendingTerm = NULL; 
    return ModuleDialog::handleLookupFinished(event, data);
}


void DreamsMainDialog::search(const char_t* term, long len)
{  
    LookupManager* lm = GetLookupManager();
    DreamsPrefs& prefs = GetPreferences()->dreamsPrefs;
    free(prefs.pendingTerm);
    prefs.pendingTerm = StringCopyN(term, len);
    if (NULL == prefs.pendingTerm)
    {
        Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
        return; 
    }    
    char* utf = UTF8_FromNative(prefs.pendingTerm);
    if (NULL == utf)
    {
        Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
        return; 
    }   
    char* url = StringCopy(urlSchemaGetDream urlSeparatorSchemaStr); 
    if (NULL == url || NULL == (url = StrAppend(url, -1, utf, -1)))
    {  
        free(utf); 
        Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
        return;
    }
    free(utf); 
    status_t err = lm->fetchUrl(url); 
    free(url);
    if (errNone != err)
        Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
}