#include "RecipesMainDialog.h"
#include "InfoManGlobals.h"
#include "HyperlinkHandler.h"
#include "Modules.h"
#include "LookupManager.h"
#include "RecipesModule.h"

#include <ByteFormatParser.hpp>
#include <Text.hpp>
#include <SysUtils.hpp>
#include <UTF8_Processor.hpp>
#include <UniversalDataFormat.hpp>

using namespace DRA;

RecipesMainDialog::RecipesMainDialog():
    listModel_(NULL),
    itemModel_(NULL),
    displayMode_(showAbout),
    query_(NULL) 
{}

RecipesMainDialog::~RecipesMainDialog()
{
    free(query_);
    delete listModel_;
    delete itemModel_;  
}

MODULE_DIALOG_CREATE_IMPLEMENT(RecipesMainDialog, IDD_RECIPES_MAIN);

bool RecipesMainDialog::handleInitDialog(HWND wnd, long lp)
{
	Rect r;
	bounds(r);
	renderer_.definition.setHyperlinkHandler(GetHyperlinkHandler());
	renderer_.definition.setInteractionBehavior(Definition::behavDoubleClickSelection | Definition::behavHyperlinkNavigation | Definition::behavMouseSelection | Definition::behavUpDownScroll);
	renderer_.definition.setNavOrderOptions(Definition::navOrderLast);
	renderer_.create(WS_TABSTOP, SCALEX(1), SCALEY(24), r.width() - SCALEX(2), r.height() - SCALEY(26), handle());
	
	term_.attachControl(handle(), IDC_SEARCH_TERM);
	back_.attachControl(handle(), IDC_BACK);
	search_.attachControl(handle(), IDC_SEARCH);
	
    setDisplayMode(displayMode_);
	
    return ModuleDialog::handleInitDialog(wnd, lp);
}

bool RecipesMainDialog::handleLookupFinished(Event& event, const LookupFinishedEventData* data)
{
    LookupManager* lm = GetLookupManager();
    UniversalDataFormat* udf = NULL; 
    switch (data->result)
    {
        case lookupResultRecipesList: 
        {
            udf = lm->releaseUDF();
            assert(NULL != udf);
            delete listModel_;
            listModel_ =  DefinitionModelFromUDF(*udf);
            delete udf; 
            if (NULL == listModel_)
                Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
            else 
                setDisplayMode(showList);
            return true;
        }

        case lookupResultRecipe:
            udf = lm->releaseUDF();
            assert(NULL != udf);
            delete itemModel_;
            itemModel_ = RecipeExtractFromUDF(*udf);
            delete udf;
            if (NULL == itemModel_)
                Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
            else
                setDisplayMode(showItem);
            return true;  

    }
    return ModuleDialog::handleLookupFinished(event, data); 
}

long RecipesMainDialog::handleCommand(ushort notify_code, ushort id, HWND sender)
{
    switch (id) {
        case IDOK:
            ModuleRunMain();
            return messageHandled;
        case IDC_SEARCH:
            search();
            return messageHandled;
        case IDC_BACK:
            setDisplayMode(showList);
            return messageHandled;
    };
    return ModuleDialog::handleCommand(notify_code, id, sender);
}

long RecipesMainDialog::handleResize(UINT sizeType, ushort width, ushort height)
{
    renderer_.anchor(anchorRight, SCALEX(2), anchorBottom, SCALEY(26), repaintWidget);
    term_.anchor(anchorRight, SCALEX(78), anchorNone, 0, repaintWidget);
    search_.anchor(anchorLeft, SCALEX(76), anchorNone, 0, repaintWidget);  
    return ModuleDialog::handleResize(sizeType, width, height);
}

void RecipesMainDialog::setDisplayMode(DisplayMode dm)
{
    switch (displayMode_ = dm)
    {
        case showAbout:
            back_.hide();
            prepareAbout(); 
            renderer_.show();
            term_.show();
            search_.show();
            term_.focus();
            break;
        
        case showList:
            back_.hide();
            assert(NULL != listModel_);
            renderer_.setModel(listModel_);
            renderer_.focus();
            term_.show();
            search_.show();
            break;

        case showItem:
            term_.hide();
            search_.hide();
            assert(NULL != itemModel_);
            renderer_.setModel(itemModel_);
            renderer_.focus();
            back_.show();
            break;
        
        default:
            assert(false);
    }  
}

void RecipesMainDialog::prepareAbout()
{
    // TODO: implement prepareAbout() 
}

void RecipesMainDialog::search()
{
    char_t* term = term_.caption();
    if (NULL == term)
    {
        Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
        return;
    }
    if (0 == Len(term))
    {
        free(term);
        return; 
    }   
    if (NULL != query_ && StrEquals(query_, term) && NULL != listModel_)
    {
        free(term); 
        setDisplayMode(showList);
        return; 
    }
    free(query_);
    query_ = term;  
    
    char* utfTerm = UTF8_FromNative(term);
    if (NULL == utfTerm)
    {
        Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
        return;
    }     
    
    char* url = StringCopy(urlSchemaRecipesList urlSeparatorSchemaStr);
    if (NULL == url)
    {
        free(utfTerm); 
        Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
        return;
    }
    
    url = StrAppend(url, -1, utfTerm, -1);
    free(utfTerm);  
    if (NULL == url)
    {
        Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
        return;
    }
    
    LookupManager* lm = GetLookupManager();
    lm->fetchUrl(url);
    free(url);     
}