#include "StocksMainDialog.h"
#include "LookupManager.h"
#include "InfoMan.h"
#include "StocksModule.h"
#include "InfoManPreferences.h"
#include "MoriartyStyles.hpp"

#include <SysUtils.hpp>
#include <Text.hpp>
#include <Utility.hpp>
#include <DefinitionStyle.hpp>

using namespace DRA;

StocksMainDialog::StocksMainDialog():
    ModuleDialog(IDR_STOCKS_MENU)
{
    setMenuBarFlags(SHCMBF_HIDESIPBUTTON);
}

StocksMainDialog::~StocksMainDialog()
{
}

MODULE_DIALOG_CREATE_IMPLEMENT(StocksMainDialog, IDD_STOCKS_MAIN)

bool StocksMainDialog::handleInitDialog(HWND fw, long ip)
{
    portfolioCombo_.attachControl(handle(), IDC_CURRENT_PORTFOLIO);
    
    list_.attachControl(handle(), IDC_STOCKS_LIST);
    portfolioValue_.attachControl(handle(), IDC_PORTFOLIO_VALUE); 
    
#ifndef LVS_EX_GRADIENT
#define LVS_EX_GRADIENT 0
#endif

    list_.setStyleEx(LVS_EX_GRADIENT | LVS_EX_FULLROWSELECT);
    list_.setTextBkColor(CLR_NONE);
    createColumns();
    
    ModuleDialog::handleInitDialog(fw, ip);
    
    portfolioCombo_.modifyStyle(CBS_DROPDOWNLIST, CBS_DROPDOWN);
    //portfolioCombo_.modifyStyle(CBS_DROPDOWN, CBS_DROPDOWNLIST);
    resyncPortfoliosCombo(); 
    resyncPortfolio(); 
    return false; 
}

long StocksMainDialog::handleResize(UINT st, ushort w, ushort h)
{
       
    // This mess below is result of resource editor using f**king "logical units"
    Widget text;
    text.attachControl(handle(), IDC_CURRENT_PORTFOLIO_TEXT);
    text.anchor(anchorNone, 0, anchorTop, h - SCALEX(8), repaintWidget);

    portfolioCombo_.anchor(anchorRight, w / 2 + SCALEX(5), anchorTop, h - SCALEX(5), repaintWidget);
    ulong_t hh = SCALEX(5) + portfolioCombo_.height() + SCALEX(5); 
    
    list_.anchor(anchorNone, 0, anchorTop, h - hh, repaintNot); 
    list_.anchor(anchorRight, 0, anchorBottom, 2 * hh, repaintWidget);
    
    text.attachControl(handle(), IDC_PORTFOLIO_VALUE_TEXT);
    text.anchor(anchorNone, 0, anchorTop, text.height() + SCALEX(7), repaintWidget); 
    
    portfolioValue_.anchor(anchorRight, w / 2 + SCALEX(5), anchorTop, portfolioValue_.height() + SCALEY(5), repaintWidget);
    return ModuleDialog::handleResize(st, w, h);
}

long StocksMainDialog::handleCommand(ushort nc, ushort id, HWND sender)
{
    StocksPrefs& prefs = GetPreferences()->stocksPrefs;
    assert(uint_t(prefs.currentPortfolio) < prefs.portfolioCount());
    StocksPortfolio& p = prefs.portfolio(prefs.currentPortfolio);
    
    switch (id) 
    { 
        case IDC_CURRENT_PORTFOLIO:
            if (CBN_SELCHANGE == nc)
            {
                long sel = portfolioCombo_.selection();
                assert(ulong_t(sel) < prefs.portfolioCount());
                prefs.currentPortfolio = sel;
                resyncPortfolio();
                return messageHandled;    
            } 
            break;
            
        case ID_STOCK_DELETE:
        {
            long index = ListView_GetNextItem(list_.handle(), -1, LVNI_ALL | LVNI_SELECTED);
            if (-1 == index)
                return messageHandled;
            assert(ulong_t(index) < p.size());
            if (IDYES != Alert(handle(), IDS_CONFIRM_STOCK_DELETE, IDS_CONFIRM, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2))
                return messageHandled;
            p.removeSymbol(index);
            list_.removeItem(index);
            return messageHandled;
        } 
        
        case ID_PORTFOLIO_DELETE:
        {
            if (1 == prefs.portfolioCount())
            {
                Alert(handle(), IDS_INFO_CANT_DELETE_LAST_PORTFOLIO, IDS_INFO, MB_OK | MB_ICONINFORMATION);
                return messageHandled;  
            }
            prefs.removePortfolio(prefs.currentPortfolio);
            prefs.currentPortfolio = 0;
            resyncPortfoliosCombo();
            resyncPortfolio();
            return messageHandled;
        }
              
    }  
    return ModuleDialog::handleCommand(nc, id, sender);
}

bool StocksMainDialog::handleLookupFinished(Event& event, const LookupFinishedEventData* data)
{
    //switch (data->result)
    //{
    //}
    return ModuleDialog::handleLookupFinished(event, data);
}

enum StocksColumn {
    scSymbol,
    scQuantity,
    scTrade,
    scChange,
    scPercentChange,
    scCount_
}; 

struct StocksColumnHeader {
    UINT textId;
    uint_t width;
};
  
static const StocksColumnHeader columnNameIds[] = 
{
    {IDS_STOCK_SYMBOL, 20},
    {IDS_STOCK_QUANTITY, 10},
    {IDS_STOCK_TRADE, 24},
    {IDS_STOCK_CHANGE, 23},
    {IDS_STOCK_PERCENT_CHANGE, 23},
};

static StaticAssert<scCount_ == ARRAY_SIZE(columnNameIds)> column_descs_valid;

void StocksMainDialog::createColumns()
{
    LVCOLUMN col;
    ZeroMemory(&col, sizeof(col));
    col.mask = LVCF_ORDER | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
    ulong_t w = list_.width();
    for (ulong_t i = 0; i < ARRAY_SIZE(columnNameIds); ++i)
    {
        char_t* text = LoadString(columnNameIds[i].textId);
        if (NULL == text)
            goto Error; 
        col.iSubItem = i;
        col.iOrder = i;
        col.pszText = text;
        if (0 != i)
        {
            col.mask |= LVCF_FMT;
            col.fmt = LVCFMT_JUSTIFYMASK | LVCFMT_RIGHT;
        }
        col.cx = (columnNameIds[i].width * w) / 100;
        int res = ListView_InsertColumn(list_.handle(), i, &col);
        free(text);
        if (-1 == res)
            goto Error;
    }  
    return; 
Error: 
    Alert(IDS_ALERT_NOT_ENOUGH_MEMORY); 
}

void StocksMainDialog::resyncPortfoliosCombo()
{
    const StocksPrefs& prefs = GetPreferences()->stocksPrefs;
    portfolioCombo_.clear();
    ulong_t size = prefs.portfolioCount();
    for (ulong_t i = 0; i < size; ++ i)
    {
        const StocksPortfolio& p = prefs.portfolio(i);
        portfolioCombo_.addString(p.name()); 
    }    
    ulong_t sel = 0;
    if (StocksPrefs::portfolioIndexInvalid != prefs.currentPortfolio)
        sel = prefs.currentPortfolio;
    portfolioCombo_.setSelection(sel);
    
    //if (1 == size)
    //    portfolioCombo_.setEnabled(false);  
}

void StocksMainDialog::resyncPortfolio()
{
    const StocksPrefs& prefs = GetPreferences()->stocksPrefs;
    assert(uint_t(prefs.currentPortfolio) < prefs.portfolioCount());
    
    list_.clear();
    const StocksPortfolio& p = prefs.portfolio(prefs.currentPortfolio);
    ulong_t size = p.size();
   
    LVITEM item;
    ZeroMemory(&item, sizeof(item));
    item.mask = LVIF_TEXT;
     
    for (ulong_t i = 0; i < size; ++i)
    {
        const StocksPortfolio::Entry& e = p.entry(i);
        char_t buffer[64];
        item.iItem = i;
        for (ulong_t j = 0; j < ARRAY_SIZE(columnNameIds); ++j)
        {
            switch (j) 
            {
                // This probably could be bold
                case scSymbol: 
                    item.pszText = e.symbol;
                    break;
                case scQuantity:
                    if (!StrStartsWith(e.symbol, _T("^")))
                    { 
                        tprintf(buffer, _T("%ld"), e.quantity);
                        localizeNumberStrInPlace(buffer);
                        item.pszText = buffer;
                    }
                    else
                        item.pszText = _T("");
                    break;
                case scTrade:
                    if (StocksPortfolio::valueNotAvailable != e.trade)
                    {
                        tprintf(buffer, _T("%.2f"), e.trade);
                        localizeNumberStrInPlace(buffer);
                        item.pszText = buffer;
                    }
                    else
                        item.pszText = _T("N/A");
                    break;
                // TODO: make these owner draw to set text color properly                    
                case scChange:
                    if (StocksPortfolio::valueNotAvailable != e.change)
                    {
                        tprintf(buffer, _T("%.2f"), e.change);
                        localizeNumberStrInPlace(buffer);
                        item.pszText = buffer;
                    }
                    else
                        item.pszText = _T("N/A");
                    break;
                case scPercentChange:
                    if (StocksPortfolio::valueNotAvailable != e.percentChange)
                    {
                        
                        tprintf(buffer, _T("%+.2f%%"), e.percentChange);
                        localizeNumberStrInPlace(buffer);
                        item.pszText = buffer;
                    }
                    else
                        item.pszText = _T("N/A");
                    break;
            }
            item.iSubItem = j;
            long res;
            if (0 == j)
                res = list_.insertItem(item);
            else
                res = ListView_SetItem(list_.handle(), &item); 
            assert(-1 != res);  
        }
    }               
}

bool StocksMainDialog::drawListViewItem(NMLVCUSTOMDRAW& data)
{
    const StocksPrefs& prefs = GetPreferences()->stocksPrefs;
    const StocksPortfolio& p = prefs.portfolio(prefs.currentPortfolio);
     
    const StocksPortfolio::Entry& e = p.entry(data.nmcd.dwItemSpec);
    if (p.valueNotAvailable != e.percentChange)
    {
        const DefinitionStyle* style = NULL; 
        if (e.percentChange < -0.001)
            style = StyleGetStaticStyle(styleNameStockPriceDown);
        else if (e.percentChange > 0.001)
            style = StyleGetStaticStyle(styleNameStockPriceUp);
        if (NULL != style)
            data.clrText = style->foregroundColor;
    }  
    return false; 
}
