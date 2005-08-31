#include "HoroscopesMainDialog.h"
#include "InfoMan.h"
#include "HoroscopesModule.h"
#include "HyperlinkHandler.h"
#include "LookupManager.h"

#include <UniversalDataHandler.hpp>
#include <SysUtils.hpp>
#include <Text.hpp>

using namespace DRA;

HoroscopesMainDialog::HoroscopesMainDialog():
    udf_(NULL),
    displayMode_(showSigns) 
{
    setMenuBarFlags(SHCMBF_HIDESIPBUTTON);
}

MODULE_DIALOG_CREATE_IMPLEMENT(HoroscopesMainDialog, IDD_HOROSCOPES_MAIN)

HoroscopesMainDialog::~HoroscopesMainDialog()
{
    delete udf_;
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

    ModuleDialog::handleInitDialog(fw, ip); 
    
    udf_ = UDF_ReadFromStream(horoscopeDataStream);
    if (NULL != udf_)
    {
        DefinitionModel* model = HoroscopeExtractFromUDF(*udf_);
        if (NULL != model)
            renderer_.setModel(model, Definition::ownModel);
    }   
    
    prepareSigns();
     
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
    // TODO: change menu etc.
    switch (displayMode_ = dm) 
    {
        case showSigns:
            renderer_.hide();
            listView_.show();
            break;
            
        case showHoroscope:
            listView_.hide();
            renderer_.show();
            break;
    }  
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
        case IDOK:
        case IDCANCEL:
            ModuleRunMain();
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
            
        uint_t i = h.iItem;
        status_t err = HoroscopeFetch(i);
        if (errNone != err)
            Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
        return messageHandled;
    }
Default:
    return ModuleDialog::handleNotify(controlId, header);            
}

bool HoroscopesMainDialog::handleLookupFinished(Event& event, const LookupFinishedEventData* data)
{
    LookupManager* lm = GetLookupManager();
    switch (data->result)
    {
        case lookupResultHoroscope:
        {
            PassOwnership(lm->udf, udf_);
            assert(NULL != udf_);
            DefinitionModel* model = HoroscopeExtractFromUDF(*udf_);
            if (NULL != model)
            {
                renderer_.setModel(model, Definition::ownModel);
                setDisplayMode(showHoroscope);
            }
            else
                Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
            return true;
        } 
    }
    return ModuleDialog::handleLookupFinished(event, data);    
}
