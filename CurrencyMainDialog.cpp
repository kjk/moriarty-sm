#include "CurrencyMainDialog.h"
#include "LookupManager.h"
#include "InfoMan.h"
#include "CurrencyModule.h"
#include "InfoManPreferences.h"

#include <Text.hpp>
#include <Currencies.hpp>
#include <SysUtils.hpp>
#include <UniversalDataFormat.hpp>

using namespace DRA;

CurrencyMainDialog::CurrencyMainDialog():
    ModuleDialog(IDR_CURRENCY_MENU)
{
}

CurrencyMainDialog::~CurrencyMainDialog()
{
}

MODULE_DIALOG_CREATE_IMPLEMENT(CurrencyMainDialog, IDD_CURRENCY_MAIN)

bool CurrencyMainDialog::handleInitDialog(HWND fw, long ip)
{
    list_.attachControl(handle(), IDC_CURRENCY_LIST);
    list_.setStyleEx(LVS_EX_FULLROWSELECT | LVS_EX_GRADIENT | LVS_EX_ONECLICKACTIVATE);
    label_.attachControl(handle(), IDC_AMOUNT_TEXT);
    edit_.attachControl(handle(), IDC_AMOUNT);

    ModuleDialog::handleInitDialog(fw, ip);
    
    createSipPrefControl();
    createListColumns();
    createListItems();
    
    edit_.focus();
    
    return false;
}

long CurrencyMainDialog::handleResize(UINT st, ushort w, ushort h)
{
    long he = edit_.height();
    long hh = h - SCALEY(5) * 2 - he;
    list_.setBounds(0, 0, w, hh, repaintWidget);
    
    long wl = label_.width();
    label_.setBounds(SCALEX(5), hh + SCALEY(7), wl, label_.height(), repaintWidget);
    wl += SCALEX(7);
    edit_.setBounds(wl, SCALEX(5) + hh, w- wl - SCALEX(5), edit_.height(), repaintWidget);
    
    return ModuleDialog::handleResize(st, w, h);
}

long CurrencyMainDialog::handleCommand(ushort nc, ushort id, HWND sender)
{
    CurrencyPrefs& prefs = GetPreferences()->currencyPrefs;
    switch (id)
    {
        case ID_CURRENCY_DELETE:
        {
            long sel = list_.selection();
            if (-1 == sel)
                return messageHandled;
            if (IDYES != Alert(handle(), IDS_CONFIRM_CURRENCY_DELETE, IDS_CONFIRM, MB_YESNO | MB_ICONQUESTION))
                return messageHandled;
            ulong_t index = prefs.selectedCurrencies[sel];
            prefs.deselectCurrency(index);
            createListItems();
            list_.focusItem(0);
            return messageHandled;
        }
    }
    return ModuleDialog::handleCommand(nc, id, sender);
}

bool CurrencyMainDialog::handleLookupFinished(Event& event, const LookupFinishedEventData* data)
{
    //switch (data->result)
    //{
    //}
    return ModuleDialog::handleLookupFinished(event, data);
}

static const struct {
    UINT textId;
    uint_t width;
} currencyColumns[] = {
    {IDS_CURRENCY_SYMBOL, 15},
    {IDS_CURRENCY, 30},
    {IDS_CURRENCY_REGION, 35},
    {IDS_CURRENCY_VALUE, 20}
};

void CurrencyMainDialog::createListColumns()
{
    LVCOLUMN col;
    ZeroMemory(&col, sizeof(col));
    col.mask = LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
    
    ulong_t w = list_.width();
    for (ulong_t i = 0; i < ARRAY_SIZE(currencyColumns); ++i)
    {
        char_t* text = LoadString(currencyColumns[i].textId);
        if (NULL == text)
            goto Error; 
        col.iSubItem = i;
        col.iOrder = i;
        col.pszText = text;
        if (3 == i)
        {
            col.mask |= LVCF_FMT;
            col.fmt = LVCFMT_JUSTIFYMASK | LVCFMT_RIGHT;
        }
        col.cx = (currencyColumns[i].width * w) / 100;
        int res = list_.insertColumn(i, col);
        free(text);
        if (-1 == res)
            goto Error;
    }  
    return; 
Error: 
    Alert(IDS_ALERT_NOT_ENOUGH_MEMORY); 
}

void CurrencyMainDialog::createListItems(bool update)
{
    long sel = list_.selection();
    if (!update)
        list_.clear();
        
    CurrencyPrefs& prefs = GetPreferences()->currencyPrefs;


    double amount = 0.0;
    char_t* t = edit_.text();
    if (NULL != t)
        numericValue(t, -1, amount);
    free(t);
    
    
    ulong_t size = prefs.selectedCurrencies.size();

    if (-1 == sel && 0 != size)
        sel = 0;
    double baseRate = 0.0;
    if (-1 != sel && NULL != prefs.udf && ulong_t(sel) < prefs.udf->getItemsCount())
        baseRate = GetCurrencyRate(*prefs.udf, prefs.selectedCurrencies[sel]);
    
    
    LVITEM item;
    ZeroMemory(&item, sizeof(item));
    for (ulong_t i = 0; i < size; ++i)
    {
        item.iItem = item.lParam = i;
        item.mask = LVIF_TEXT;
        uint_t index = prefs.selectedCurrencies[i];
        const char_t* symbol = CurrencySymbol(index);
        
        if (!update) 
        {
            item.mask = LVIF_PARAM | LVIF_TEXT;
            if (sel == i)
            {
                item.mask |= LVIF_STATE;
                item.state = LVIS_SELECTED | LVIS_FOCUSED;
            }
            else
                item.state = 0;
            item.pszText = const_cast<char_t*>(symbol);
            item.iSubItem = 0;
            DTEST(-1 != list_.insertItem(item));
            item.mask = LVIF_TEXT;
            
            item.iSubItem = 1;
            item.pszText = const_cast<char_t*>(CurrencyName(index));
            DTEST(list_.setItem(item));
            
            item.iSubItem = 2;
            item.pszText = const_cast<char_t*>(CurrencyRegion(index));
            DTEST(list_.setItem(item));
        }
        
        item.iSubItem = 3;
        item.pszText = _T("N/A");
        if (NULL != prefs.udf && i < prefs.udf->getItemsCount())
        {
            double rate = GetCurrencyRate(*prefs.udf, symbol);
            if (0.0 != rate && 0.0 != baseRate)
            {
                double val = rate * amount/baseRate;
                if (val < 0.01)
                    val = 0.0;
                char_t buffer[32];
                tprintf(buffer, _T("%.2f"), val);
                StrNumberApplyGrouping(buffer, 32);
                localizeNumberStrInPlace(buffer);
                item.pszText = buffer;
            }
        }
        DTEST(list_.setItem(item));
    }  
}

long CurrencyMainDialog::handleNotify(int controlId, const NMHDR &header)
{
    if (IDC_CURRENCY_LIST == controlId && LVN_ITEMCHANGED == header.code && handleListItemChanged((NMLISTVIEW&)header))
        return messageHandled;

    return ModuleDialog::handleNotify(controlId, header);
}

bool CurrencyMainDialog::handleListItemChanged(NMLISTVIEW &lv)
{
    if (0 == (LVIF_STATE & lv.uChanged))
        return false;

    if (0 == (LVIS_SELECTED & lv.uNewState))
        return false;
        
    
    return false;
}

LRESULT CurrencyMainDialog::callback(UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (WM_CONTEXTMENU == msg && handleContextMenu(wParam, lParam))
        return messageHandled;

    return ModuleDialog::callback(msg, wParam, lParam);
}

bool CurrencyMainDialog::handleContextMenu(WPARAM wParam, LPARAM lParam)
{
    if (list_.handle() != (HWND)wParam)
        return false;
    
    HMENU menu = menuBar().subMenu(IDM_CURRENCY); 
    if (NULL == menu)
        return false;
        
    bool onlyAdd = false;

    Point p(LOWORD(lParam), HIWORD(lParam));
    if (-1 == p.x && -1 == p.y)
    {
        long item = list_.selection();
        if (-1 == item)
            return false;
              
        Rect rect;
        if (!list_.itemBounds(item, rect, LVIR_SELECTBOUNDS))
            return false;
            
        ClientToScreen(list_.handle(), rect);
        rect.center(p);
    }
    else
    {
        LVHITTESTINFO ht;
        ht.pt = p;
        ScreenToClient(list_.handle(), &ht.pt);
        list_.hitTest(ht);
        if (0 == (LVHT_ONITEM & ht.flags))
            onlyAdd = true;
        else
            list_.setSelection(ht.iItem);
    }
    
    if (onlyAdd)
    {
        EnableMenuItem(menu, ID_CURRENCY_DELETE, MF_GRAYED); 
    }   
     
    TrackPopupMenu(menu, TPM_TOPALIGN | TPM_LEFTALIGN, p.x, p.y, 0, handle(), NULL);
   
    if (onlyAdd)
    {
        EnableMenuItem(menu, ID_CURRENCY_DELETE, MF_ENABLED); 
    }
    
    return true;
}
