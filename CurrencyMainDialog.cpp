#include "CurrencyMainDialog.h"
#include "LookupManager.h"
#include "InfoMan.h"
#include "CurrencyModule.h"
#include "InfoManPreferences.h"

#include <Text.hpp>
#include <Currencies.hpp>
#include <SysUtils.hpp>

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
    {IDS_CURRENCY, 40},
    {IDS_CURRENCY_REGION, 40},
    {IDS_CURRENCY_VALUE, 25}
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

void CurrencyMainDialog::createListItems()
{
    list_.clear();
    CurrencyPrefs& prefs = GetPreferences()->currencyPrefs;

    
    LVITEM item;
    ZeroMemory(&item, sizeof(item));
    item.mask = LVIF_PARAM | LVIF_TEXT;
    ulong_t size = prefs.selectedCurrencies.size();
    for (ulong_t i = 0; i < size; ++i)
    {
        item.iItem = item.lParam = i;
        uint_t index = prefs.selectedCurrencies[i];
        const char_t* symbol = CurrencySymbol(index);;
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
        
        item.iSubItem = 3;
        item.pszText = _T("N/A");
        if (NULL != prefs.udf)
        {
            double rate = GetCurrencyRate(*prefs.udf, symbol);
            char_t buffer[32];
            tprintf(buffer, _T("%.2f"), rate);
            StrNumberApplyGrouping(buffer, 32);
            localizeNumberStrInPlace(buffer);
            item.pszText = buffer;
        }
        DTEST(list_.setItem(item));
    }  
}