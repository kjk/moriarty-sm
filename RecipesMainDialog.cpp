#include "RecipesMainDialog.h"
#include "InfoManGlobals.h"
#include "HyperlinkHandler.h"
#include "Modules.h"
#include "LookupManager.h"
#include "RecipesModule.h"
#include "InfoManPreferences.h"

#include <ByteFormatParser.hpp>
#include <Text.hpp>
#include <SysUtils.hpp>
#include <UTF8_Processor.hpp>
#include <UniversalDataFormat.hpp>

using namespace DRA;

static UINT recipePrefsCheckboxes[] = {
    IDC_RECIPE_NAME,
    IDC_RECIPE_NOTE,
    IDC_RECIPE_INGREDIENTS,
    IDC_RECIPE_PREPARATION,
    IDC_RECIPE_REVIEWS,
    IDC_RECIPE_GLOBAL_NOTE
};

static const StaticAssert<ARRAY_SIZE(recipePrefsCheckboxes) == RecipesPrefs::recipeSectionsCount_> ids_count_equals_sections_count;

class RecipesPrefsDialog: public MenuDialog {

    RecipesPrefsDialog()
    {
        setAutoDelete(autoDeleteNot); 
        setMenuBarFlags(SHCMBF_HIDESIPBUTTON);
    }
   
    ~RecipesPrefsDialog();
   
public:
           
    static long showModal(HWND parent);
 
protected:

    bool handleInitDialog(HWND focus_widget_handle, long init_param)
    {
        const RecipesPrefs& prefs = GetPreferences()->recipesPrefs;
        Button b;
        for (uint_t i = 0; i < prefs.recipeSectionsCount_; ++i)
        {
            b.attachControl(handle(), recipePrefsCheckboxes[i]);
            assert(b.valid());
            b.setCheck(prefs.activeSections[i] ? BST_CHECKED : BST_UNCHECKED);
        }
        b.detach();
        return MenuDialog::handleInitDialog(focus_widget_handle, init_param);
    }
   
	long handleCommand(ushort notify_code, ushort id, HWND sender);
	
};

long RecipesPrefsDialog::showModal(HWND parent)
{
    RecipesPrefsDialog dlg;
    return dlg.Dialog::showModal(GetInstance(), MAKEINTRESOURCE(IDD_RECIPES_PREFS), parent); 
}  


long RecipesPrefsDialog::handleCommand(ushort notify_code, ushort id, HWND sender)
{
	switch (id) 
	{
	    case IDOK:
        {
            RecipesPrefs& prefs = GetPreferences()->recipesPrefs;
            Button b;
            for (uint_t i = 0; i < prefs.recipeSectionsCount_; ++i)
            {
                b.attachControl(handle(), recipePrefsCheckboxes[i]);
                assert(b.valid());
                prefs.activeSections[i] = (BST_CHECKED == b.checked());    
            }
        }
        // Intentional fall-through
        case IDCANCEL:
            endModal(id);
            return messageHandled; 
    }
    return Dialog::handleCommand(notify_code, id, sender);
}


RecipesPrefsDialog::~RecipesPrefsDialog()
{
}   


RecipesMainDialog::RecipesMainDialog():
    ModuleDialog(IDR_RECIPES_MENU),
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

MODULE_DIALOG_SHOW_MODAL_IMPLEMENT(RecipesMainDialog, IDD_RECIPES_MAIN);

void RecipesMainDialog::resyncViewMenu()
{
    HMENU menu = menuBar().subMenu(IDM_VIEW);
    EnableMenuItem(menu, ID_VIEW_RECIPES_LIST, (NULL == listModel_ ? MF_GRAYED : MF_ENABLED));
    EnableMenuItem(menu, ID_VIEW_RECIPE, (NULL == itemModel_ ? MF_GRAYED : MF_ENABLED));
    CheckMenuRadioItem(menu,  ID_VIEW_RECIPE, ID_VIEW_RECIPES_LIST, (showList == displayMode_ ? ID_VIEW_RECIPES_LIST : ID_VIEW_RECIPE), MF_BYCOMMAND);
    //CheckMenuItem(menu, ID_VIEW_RECIPES_LIST, (showList == displayMode_ ? MF_CHECKED : MF_UNCHECKED));
    //CheckMenuItem(menu, ID_VIEW_RECIPE, (showItem == displayMode_ ? MF_CHECKED : MF_UNCHECKED));
}

bool RecipesMainDialog::handleInitDialog(HWND wnd, long lp)
{
    createSipPrefControl();
    
	term_.attachControl(handle(), IDC_SEARCH_TERM);
	
	Rect r;
	innerBounds(r);
	renderer_.definition.setHyperlinkHandler(GetHyperlinkHandler());
	renderer_.definition.setInteractionBehavior(Definition::behavDoubleClickSelection | Definition::behavHyperlinkNavigation | Definition::behavMouseSelection | Definition::behavUpDownScroll);
	renderer_.definition.setNavOrderOptions(Definition::navOrderLast);
	
	long h = term_.height() + LogY(2);
	renderer_.create(WS_TABSTOP | WS_VISIBLE, LogX(1), h, r.width() - 2 * LogX(1), r.height() - h - LogY(1), handle());
	
	ModuleDialog::handleInitDialog(wnd, lp);
	overrideBackKey();
	
    RecipesDataRead(listModel_, itemModel_);
	if (NULL != itemModel_)
	    setDisplayMode(showItem);
    else if (NULL != listModel_)
        setDisplayMode(showList);
    else 
        setDisplayMode(displayMode_);

    return false;
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
    switch (id) 
    {
        case ID_SEARCH:
            search();
            return messageHandled;
            
        case ID_VIEW_RECIPES_LIST:
        case ID_BACK:
            setDisplayMode(showList);
            return messageHandled;
            
        case ID_VIEW_RECIPE:
            setDisplayMode(showItem);
            return messageHandled;

        case ID_VIEW_PREFERENCES:
            if (IDOK == RecipesPrefsDialog::showModal(handle()))
            {
                if (showItem == displayMode_ || showList == displayMode_)
                    renderer_.setModel(NULL);
                     
                RecipesDataRead(listModel_, itemModel_);
                setDisplayMode(displayMode_); 
            }
            return messageHandled;
            
    };
    return ModuleDialog::handleCommand(notify_code, id, sender);
}

long RecipesMainDialog::handleResize(UINT sizeType, ushort width, ushort height)
{
    term_.anchor(anchorRight, 2 * LogX(1), anchorNone, 0, repaintWidget);
    long h = term_.height() + LogY(2); 
    if (showItem == displayMode_)
        renderer_.anchor(anchorRight, 2 * LogX(1), anchorBottom, 2 * LogY(1), repaintWidget);
    else
        renderer_.anchor(anchorRight, 2 * LogX(1), anchorBottom, h + LogY(1), repaintWidget);
        
    return ModuleDialog::handleResize(sizeType, width, height);
}

void RecipesMainDialog::setDisplayMode(DisplayMode dm)
{
    Rect b;
    bounds(b);
    long h = term_.height() + LogY(2); 
    b.set(LogX(1), h, b.width() - 2 * LogX(1), b.height() - h - LogY(1));
    switch (displayMode_ = dm)
    {
        case showAbout:
            prepareAbout(); 
            renderer_.setBounds(b);
            menuBar().replaceButton(ID_BACK, ID_SEARCH, IDS_SEARCH);
            term_.show();
            term_.focus();
            break;
        
        case showList:
            assert(NULL != listModel_);
            renderer_.setBounds(b);
            renderer_.setModel(listModel_);
            renderer_.focus();
            menuBar().replaceButton(ID_BACK, ID_SEARCH, IDS_SEARCH);
            term_.show();
            break;

        case showItem:
            term_.hide();
            assert(NULL != itemModel_);
            b.y() = LogY(1);
            renderer_.setBounds(b);
            renderer_.setModel(itemModel_);
            renderer_.focus();
            menuBar().replaceButton(ID_SEARCH, ID_BACK, IDS_BACK);
            break;
        
        default:
            assert(false);
    }  
    resyncViewMenu();
}

void RecipesMainDialog::prepareAbout()
{
	ByteFormatParser* parser = NULL;
	DefinitionModel* model = NULL;
	status_t err = errNone;
    const char* data = (const char*)LoadBinaryData(IDR_RECIPES_START);
    if (NULL == data)
        goto Error;

    parser = new_nt ByteFormatParser();
    if (NULL == parser)
        goto Error;
        
    err = parser->parseAll(data, -1); 
    if (memErrNotEnoughSpace == err)
        goto Error;
    assert(errNone == err);
   
    model = parser->releaseModel();
    if (NULL == model)
        goto Error;
    
    renderer_.setModel(model, Definition::ownModel);      
Finish:
    if (NULL != parser)
        delete parser; 
    return; 
Error:
    Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
    ModuleRunMain(); 
    goto Finish;         
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