#include "HoroscopesMainDialog.h"
#include "InfoMan.h"
#include "HoroscopesModule.h"
#include "HyperlinkHandler.h"
#include "LookupManager.h"
#include "InfoManPreferences.h"

#include <UniversalDataHandler.hpp>
#include <SysUtils.hpp>
#include <Text.hpp>

using namespace DRA;

HoroscopesMainDialog::HoroscopesMainDialog():
    ModuleDialog(IDR_HOROSCOPES_MENU),
    udf_(NULL),
    displayMode_(showSigns),
    date_(NULL)
{
    setMenuBarFlags(SHCMBF_HIDESIPBUTTON);
}

MODULE_DIALOG_CREATE_IMPLEMENT(HoroscopesMainDialog, IDD_HOROSCOPES_MAIN)

HoroscopesMainDialog::~HoroscopesMainDialog()
{
    delete udf_;
    free(date_); 
}

bool HoroscopesMainDialog::handleInitDialog(HWND fw, long ip)  
{
    listView_.attachControl(handle(), IDC_LIST);

#ifndef LVS_EX_GRADIENT
#define LVS_EX_GRADIENT 0
#endif

    listView_.setStyleEx(LVS_EX_GRADIENT | LVS_EX_ONECLICKACTIVATE | LVS_EX_NOHSCROLL);
    listView_.setTextBkColor(CLR_NONE);
    
    Rect r;
    innerBounds(r);  
    renderer_.create(WS_TABSTOP | WS_VISIBLE, LogX(1), LogY(1), r.width() - 2 * LogX(1), r.height() - 2 * LogY(1), handle());
    renderer_.definition.setInteractionBehavior(
        Definition::behavUpDownScroll
      | Definition::behavHyperlinkNavigation
      | Definition::behavMouseSelection
    );  

    ModuleDialog::handleInitDialog(fw, ip); 
    
    udf_ = UDF_ReadFromStream(horoscopeDataStream);
    if (NULL != udf_)
    {
        DefinitionModel* model = HoroscopeExtractFromUDF(*udf_, date_);
        if (NULL != model)
            renderer_.setModel(model, Definition::ownModel);
    }   
    
    prepareSigns();
    HoroscopesPrefs& prefs = GetPreferences()->horoscopesPrefs;
    ulong_t index = 0;
    if (prefs.signNotSet != prefs.finishedSign)
        index = prefs.finishedSign;
    ListView_SetItemState(listView_.handle(), index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
     
    setDisplayMode(displayMode_);
    return false;  
}

long HoroscopesMainDialog::handleResize(UINT sizeType, ushort width, ushort height)
{
    listView_.anchor(anchorRight, 2 * LogX(1), anchorBottom, 2 * LogY(1), repaintWidget);
    renderer_.anchor(anchorRight, 2 * LogX(1), anchorBottom, 2 * LogY(1), repaintWidget);
    return ModuleDialog::handleResize(sizeType, width, height); 
}

void HoroscopesMainDialog::setDisplayMode(DisplayMode dm)
{
    switch (displayMode_ = dm) 
    {
        case showSigns:
            renderer_.hide();
            listView_.show();
            menuBar().replaceButton(ID_BACK, IDOK, IDS_DONE);
            listView_.focus();
            break;
            
        case showHoroscope:
            listView_.hide();
            renderer_.show();
            menuBar().replaceButton(IDOK, ID_BACK, IDS_BACK);
            renderer_.focus();
            break;
    }
    resyncViewMenu();   
}

void HoroscopesMainDialog::prepareSigns()
{
    bool scaleIcons = false;
    if (SCALEX(50) >= 100)
        scaleIcons = true; 

    listView_.clear();   
    for (ulong_t i = 0; i < horoscopesSignCount; ++i)
    {
        char_t* name = LoadString(IDS_ZODIAC0 + i);
        if (NULL == name)
            goto Error;
        
        LVITEM item;
        ZeroMemory(&item, sizeof(item));

        item.mask = LVIF_TEXT; // | LVIF_IMAGE;
        item.iItem = i;
        item.iSubItem = 0;
        item.pszText = name;
        // item.iImage = item.iItem;
        
        long res = listView_.insertItem(item);
        free(name);
        if (-1 == res)
            goto Error;
    }
    return;
Error:
    assert(false); 
}

long HoroscopesMainDialog::handleCommand(ushort nc, ushort id, HWND sender)
{
    switch (id)
    {
        case ID_VIEW_SIGN_LIST:
        case ID_BACK:
            setDisplayMode(showSigns);
            return messageHandled;

        case ID_VIEW_HOROSCOPE:
            setDisplayMode(showHoroscope);
            return messageHandled;
        
        case ID_VIEW_UPDATE:
            if (errNone != HoroscopeFetch(GetPreferences()->horoscopesPrefs.finishedQuery))
               Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
            return messageHandled;
    }
    return ModuleDialog::handleCommand(nc, id, sender);   
}

long HoroscopesMainDialog::handleNotify(int controlId, const NMHDR& header)
{
    if  (LVN_ITEMACTIVATE == header.code)
    {
        const NMLISTVIEW& h = (const NMLISTVIEW&)header;
        if (-1 == h.iItem)
            goto Default;
            
        status_t err = HoroscopeFetch(h.iItem);
        if (errNone != err)
            Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
        return messageHandled;
    }
Default:
    return ModuleDialog::handleNotify(controlId, header);            
}

bool HoroscopesMainDialog::handleLookupFinished(Event& event, const LookupFinishedEventData* data)
{
    HoroscopesPrefs& prefs = GetPreferences()->horoscopesPrefs;
    LookupManager* lm = GetLookupManager();
    switch (data->result)
    {
        case lookupResultHoroscope:
        {
            PassOwnership(lm->udf, udf_);
            assert(NULL != udf_);
            DefinitionModel* model = HoroscopeExtractFromUDF(*udf_, date_);
            if (NULL != model)
            {
                renderer_.setModel(model, Definition::ownModel);
                ModuleTouchRunning();
                assert(NULL != prefs.pendingQuery);
                free(prefs.finishedQuery);
                prefs.finishedQuery = prefs.pendingQuery;
                prefs.pendingQuery = NULL;
                
                if (prefs.signNotSet != prefs.pendingSign)
                    prefs.finishedSign = prefs.pendingSign;                
                                           
                setDisplayMode(showHoroscope);
            }
            else
                Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
            free(prefs.pendingQuery);
            prefs.pendingQuery = NULL; 
            prefs.pendingSign = prefs.signNotSet; 
            return true;
        } 
    }
    free(prefs.pendingQuery);
    prefs.pendingQuery = NULL; 
    prefs.pendingSign = prefs.signNotSet; 
    return ModuleDialog::handleLookupFinished(event, data);    
}

void HoroscopesMainDialog::resyncViewMenu()
{
    HoroscopesPrefs& prefs = GetPreferences()->horoscopesPrefs;
    HMENU menu = menuBar().subMenu(IDM_VIEW);
    EnableMenuItem(menu, ID_VIEW_HOROSCOPE, (renderer_.definition.empty() ? MF_GRAYED : MF_ENABLED));
    CheckMenuItem(menu, ID_VIEW_SIGN_LIST, (showSigns == displayMode_ ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, ID_VIEW_HOROSCOPE, (showHoroscope == displayMode_ ? MF_CHECKED : MF_UNCHECKED));
    EnableMenuItem(menu, ID_VIEW_UPDATE, (0 == Len(prefs.finishedQuery) ? MF_GRAYED : MF_ENABLED));
}