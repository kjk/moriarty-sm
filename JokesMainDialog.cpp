#include "JokesMainDialog.h"
#include "LookupManager.h"
#include "InfoMan.h"
#include "InfoManPreferences.h"

#include <SysUtils.hpp>
#include <UniversalDataHandler.hpp>

class JokesSearchDialog: public MenuDialog {

    
};

JokesMainDialog::JokesMainDialog():
    ModuleDialog(IDR_JOKES_MENU),
    displayMode_(showJoke)
{
    setMenuBarFlags(SHCMBF_HIDESIPBUTTON);
    renderer_.definition.setInteractionBehavior(Definition::behavUpDownScroll | Definition::behavMouseSelection);
}

JokesMainDialog::~JokesMainDialog()
{
}

MODULE_DIALOG_CREATE_IMPLEMENT(JokesMainDialog, IDD_JOKES_MAIN) 

bool JokesMainDialog::handleInitDialog(HWND fw, long ip)
{
    Rect r;
    innerBounds(r);
    renderer_.create(WS_TABSTOP, r, handle(), NULL);
    list_.attachControl(handle(), IDC_JOKE_LIST);

    ModuleDialog::handleInitDialog(fw, ip);
    setDisplayMode(displayMode_);
    
    bool empty = true;
    JokesPrefs& prefs = GetPreferences()->jokesPrefs;
    if (NULL == prefs.udf)
    {
        prefs.udf = UDF_ReadFromStream(jokesJokesListStream);
        if (NULL != prefs.udf)
        {
            setDisplayMode(showList);
            empty = false;
        }
    }
    
    UniversalDataFormat* udf = UDF_ReadFromStream(jokesJokeStream);
    if (NULL != udf)
    {
        DefinitionModel* model = JokeExtractFromUDF(*udf);
        if (NULL != model)
        {
            renderer_.setModel(model, Definition::ownModel);
            setDisplayMode(showJoke);
            empty = false;
        }
    }
    
    if (empty && errNone != JokesFetchRandom())
        Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
    
    return false;
}

void JokesMainDialog::setDisplayMode(DisplayMode dm)
{
    switch (displayMode_ = dm)
    {
        case showJoke:
            list_.hide();
            renderer_.show();
            renderer_.focus();
            break;
        case showList:
        {
            renderer_.hide();
            list_.show();
            list_.focus();
            long sel = list_.selection();
            if (-1 == sel)
                sel = 0;
            if (0 != list_.itemCount())
                list_.focusItem(sel);
            break;
        }
    }
    resyncViewMenu();
}

void JokesMainDialog::resyncViewMenu()
{
    HMENU menu = menuBar().subMenu(IDM_VIEW);
    CheckMenuRadioItem(menu, ID_VIEW_JOKE, ID_VIEW_JOKE_LIST, (showJoke == displayMode_ ? ID_VIEW_JOKE : ID_VIEW_JOKE_LIST), MF_BYCOMMAND);
    JokesPrefs& prefs = GetPreferences()->jokesPrefs;
    UINT state = MF_BYCOMMAND | (NULL == prefs.udf ? MF_GRAYED : MF_ENABLED);
    EnableMenuItem(menu, ID_VIEW_JOKE_LIST, state);
    state = MF_BYCOMMAND | (renderer_.definition.empty() ? MF_GRAYED : MF_ENABLED);
    EnableMenuItem(menu, ID_VIEW_JOKE, state);
}

long JokesMainDialog::handleResize(UINT st, ushort w, ushort h)
{
    renderer_.anchor(anchorRight, 0, anchorBottom, 0, repaintWidget);
    list_.anchor(anchorRight, 0, anchorBottom, 0, repaintWidget);
    
    return messageHandled;
}

long JokesMainDialog::handleCommand(ushort nc, ushort id, HWND sender)
{
    switch (id) 
    {
        case IDM_JOKES_RANDOM:
            if (errNone != JokesFetchRandom())
                Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
            return messageHandled;
            
        case ID_VIEW_JOKE:
            if (showJoke != displayMode_)
                setDisplayMode(showJoke);
            return messageHandled;

        case ID_VIEW_JOKE_LIST:
            if (showList != displayMode_)
                setDisplayMode(showList);
            return messageHandled;
            
    }
    return ModuleDialog::handleCommand(nc, id, sender);
}

bool JokesMainDialog::handleLookupFinished(Event& event, const LookupFinishedEventData* data)
{
    JokesPrefs& prefs = GetPreferences()->jokesPrefs;
    LookupManager& lm = *GetLookupManager();
    UniversalDataFormat* udf = NULL;
    switch (data->result)
    {
        case lookupResultJokesList:
            PassOwnership(lm.udf, prefs.udf);
            assert(NULL != prefs.udf);
            createListItems();
            setDisplayMode(showList);
            return true;
        
        case lookupResultJoke:
        {
            PassOwnership(lm.udf, udf);
            assert(NULL != udf);
            DefinitionModel* model = JokeExtractFromUDF(*udf);
            delete udf;
            if (NULL == model)
            {
                Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
                return true;
            }
            renderer_.setModel(model, Definition::ownModel);
            setDisplayMode(showJoke);
            return true;
        }
    }
    return ModuleDialog::handleLookupFinished(event, data);
}

void JokesMainDialog::createListColumns()
{
}

void JokesMainDialog::createListItems()
{
}