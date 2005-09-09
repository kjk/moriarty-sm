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

class StocksEditPortfolioDialog: public MenuDialog {

    EditBox edit_;
    Widget infoText_; 
    long index_;
    char_t* name_;
     
    StocksEditPortfolioDialog(long index):
        MenuDialog(IDR_DONE_CANCEL_MENU, true),
        index_(index),
        name_(NULL)
    {
        setAutoDelete(autoDeleteNot); 
    }
   
    ~StocksEditPortfolioDialog()
    {
    }
    
protected:
    
    bool handleInitDialog(HWND fw, long ip)
    {   
        infoText_.attachControl(handle(), IDC_PORTFOLIO_INFO_TEXT); 
        edit_.attachControl(handle(), IDC_PORTFOLIO_NAME); 
        if (-1 != index_)
        {
            const StocksPortfolio& p = GetPreferences()->stocksPrefs.portfolio(index_);
            edit_.setCaption(p.name());
            edit_.setSelection();
            infoText_.hide();
        } 
        else
            infoText_.show();
            
        edit_.focus();
        
        MenuDialog::handleInitDialog(fw, ip);
        
        return false;
    }

    long handleCommand(ushort nc, ushort id, HWND sender)
    {
        switch (id)
        {
            case IDOK:
                free(name_);
                name_ = edit_.caption();
                {
                    ulong_t len = Len(name_);
                    const char_t* n = name_;
                    strip(n, len);
                    if (0 == len)
                    {
                        edit_.setSelection();
                        edit_.focus();
                        return messageHandled;
                    }
                }
                // intentional fall-through 
            case IDCANCEL: 
                endModal(id);
                return messageHandled;
        }
        return MenuDialog::handleCommand(nc, id, sender); 
    }   

public:
    
    static long showModal(char_t*& name, HWND parent, long index)
    {
        StocksEditPortfolioDialog dlg(index);
        long res = dlg.MenuDialog::showModal(GetInstance(), IDD_STOCKS_EDIT_PORTFOLIO, parent);
        if (IDOK == res)
        {
            free(name);
            name = dlg.name_;
            dlg.name_ = NULL;
        }
        return res;
    }
    
};

class StocksEditStockDialog: public MenuDialog {
    
    EditBox symbolBox_;
    EditBox quantityBox_;
     
    long index_;
    char_t* symbol_;
    ulong_t quantity_;

    StocksEditStockDialog(long index):
        MenuDialog(IDR_DONE_CANCEL_MENU, true),
        symbol_(NULL),
        quantity_(0),
        index_(index)
    {
        setAutoDelete(autoDeleteNot); 
    }
    
    ~StocksEditStockDialog()
    {
        free(symbol_); 
    }

protected:
    
    bool handleInitDialog(HWND fw, long ip)
    {   
        symbolBox_.attachControl(handle(), IDC_STOCK_SYMBOL);
        quantityBox_.attachControl(handle(), IDC_STOCK_QUANTITY);
        
        if (-1 != index_)
        {
            const StocksPrefs& prefs = GetPreferences()->stocksPrefs;
            const StocksPortfolio& p = prefs.portfolio(prefs.currentPortfolio);
            const StocksPortfolio::Entry& e = p.entry(index_);
            symbolBox_.setCaption(e.symbol);
            symbolBox_.setReadOnly(true);
            char_t buffer[32];
            tprintf(buffer, _T("%ld"), e.quantity);
            quantityBox_.setCaption(buffer);
            quantityBox_.setSelection();
            quantityBox_.focus();
        } 
        else
        {
            symbolBox_.focus();
            quantityBox_.setCaption(_T("0"));
        }
        
        MenuDialog::handleInitDialog(fw, ip);
        
        return false;
    }

    long handleCommand(ushort nc, ushort id, HWND sender)
    {
        switch (id)
        {
            case IDOK:
                if (-1 == index_)
                { 
                    free(symbol_);
                    symbol_ = symbolBox_.caption();
                    {
                        ulong_t len = Len(symbol_);
                        const char_t* n = symbol_;
                        strip(n, len);
                        if (0 == len)
                        {
                            symbolBox_.setSelection();
                            symbolBox_.focus();
                            return messageHandled;
                        }
                    }
                }
                { 
                    char_t* q = quantityBox_.caption();
                    long val;
                    if (NULL == q || errNone != numericValue(q, Len(q), val))
                    {
                        free(q);
                        quantityBox_.focus();
                        return messageHandled; 
                    }
                    free(q); 
                    quantity_ = val; 
                }
                // intentional fall-through 
            case IDCANCEL: 
                endModal(id);
                return messageHandled;
        }
        return MenuDialog::handleCommand(nc, id, sender); 
    }   

public:

    static long showModal(char_t*& symbol, ulong_t& quantity, HWND parent, long index)
    {
        StocksEditStockDialog dlg(index);
        long res = dlg.MenuDialog::showModal(GetInstance(), IDD_STOCKS_EDIT_STOCK, parent);
        if (IDOK == res)
        {
            free(symbol);
            symbol = dlg.symbol_;
            dlg.symbol_ = NULL;
            quantity = dlg.quantity_;
        }
        return res;
    }  
      
};
         

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
    // portfolioCombo_.setStyle(CBS_DROPDOWNLIST | WS_TABSTOP | WS_VISIBLE);
    
    list_.attachControl(handle(), IDC_STOCKS_LIST);
    portfolioValue_.attachControl(handle(), IDC_PORTFOLIO_VALUE); 
    portfolioValue_.modifyStyle(ES_RIGHT, ES_LEFT | ES_CENTER);
    
#ifndef LVS_EX_GRADIENT
#define LVS_EX_GRADIENT 0
#endif

    list_.setStyleEx(LVS_EX_GRADIENT | LVS_EX_FULLROWSELECT);
    list_.setTextBkColor(CLR_NONE);
    createColumns();
    
    ModuleDialog::handleInitDialog(fw, ip);
    
    resyncPortfoliosCombo(); 
    resyncPortfolio(); 
    return false; 
}

long StocksMainDialog::handleResize(UINT st, ushort w, ushort h)
{
    ulong_t hh = SCALEY(5) + portfolioCombo_.height() + SCALEY(5); 
       
    // This mess below is result of resource editor using f**king "logical units"
    Widget text;
    text.attachControl(handle(), IDC_CURRENT_PORTFOLIO_TEXT);
    text.setBounds(SCALEX(5), SCALEY(8), w / 2 - SCALEX(5), text.height(), repaintWidget);
    
    portfolioCombo_.setBounds(w / 2 + 1, SCALEY(5), w / 2 - SCALEX(5), portfolioCombo_.height(), repaintWidget);
    
    list_.setBounds(0, hh, w, h - 2 * hh, repaintWidget);
    hh += list_.height(); 
    
    text.attachControl(handle(), IDC_PORTFOLIO_VALUE_TEXT);
    text.setBounds(SCALEX(5), hh + SCALEY(8), w / 2 - SCALEX(5), text.height(), repaintWidget);

    portfolioValue_.setBounds(w / 2 + 1, hh + SCALEY(5), w / 2 - SCALEX(5), portfolioValue_.height());
    return ModuleDialog::handleResize(st, w, h);
}

long StocksMainDialog::handleCommand(ushort nc, ushort id, HWND sender)
{
    StocksPrefs& prefs = GetPreferences()->stocksPrefs;
    assert(uint_t(prefs.currentPortfolio) < prefs.portfolioCount());
    StocksPortfolio& p = prefs.portfolio(prefs.currentPortfolio);
    char_t* name = NULL;
    
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
        
        case ID_STOCK_SET_QUANTITY:
        {
            long index = ListView_GetNextItem(list_.handle(), -1, LVNI_ALL | LVNI_SELECTED);
            if (-1 == index)
                return messageHandled;
            assert(ulong_t(index) < p.size());
            if (StrStartsWith(p.entry(index).symbol, _T("^")))
            {
                // TODO: add info that you can't set quantity of cumulative indexes
                return messageHandled; 
            }
            ulong_t q;
            if (IDOK == StocksEditStockDialog::showModal(name, q, handle(), index))
            {
                p.entry(index).quantity = q;
                resyncPortfolio();
            }
            return messageHandled; 
        }
        
        case ID_STOCK_ADD:
        {
            ulong_t q;
            if (IDOK == StocksEditStockDialog::showModal(name, q, handle(), -1))
                addStock(name, q);
            free(name);
            return messageHandled;
        } 
            
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
            if (IDYES != Alert(handle(), IDS_CONFIRM_PORTFOLIO_DELETE, IDS_CONFIRM, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2))
                return messageHandled; 
            prefs.removePortfolio(prefs.currentPortfolio);
            prefs.currentPortfolio = 0;
            resyncPortfoliosCombo();
            resyncPortfolio();
            return messageHandled;
        }
        
        case ID_PORTFOLIO_RENAME:
            if (IDOK == StocksEditPortfolioDialog::showModal(name, handle(), prefs.currentPortfolio) && NULL != name)
            {
                p.replaceName(name);
                resyncPortfoliosCombo(); 
            }
            return messageHandled;
        
        case ID_PORTFOLIO_ADD:
            if (IDOK == StocksEditPortfolioDialog::showModal(name, handle(), -1) && NULL != name)
                createPortfolio(name);
            free(name);
            return messageHandled;
              
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
    
    char_t buffer[64];
    double value = 0; 
    
    for (ulong_t i = 0; i < size; ++i)
    {
        const StocksPortfolio::Entry& e = p.entry(i);
        if (p.valueNotAvailable != e.trade && p.valueNotAvailable != value)
            value += e.quantity * e.trade;
        else
            value = p.valueNotAvailable;

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
    if (p.valueNotAvailable == value)
        portfolioValue_.setCaption(_T("N/A"));
    else
    {
        tprintf(buffer, _T("%.2f"), value);
        portfolioValue_.setCaption(buffer); 
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

void StocksMainDialog::createPortfolio(const char_t* name)
{
    StocksPrefs& prefs = GetPreferences()->stocksPrefs;
    StocksPortfolio* p = prefs.addPortfolio(name);
    if (NULL == p)
    {
        Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
        return; 
    }
    prefs.currentPortfolio = prefs.portfolioCount() - 1;
    resyncPortfoliosCombo();
    ulong_t size = p->size();
    ulong_t synced = 0; 
    for (ulong_t i = 0; i < size; ++i)
        if (StocksResyncEntry(p->entry(i), prefs.currentPortfolio))
            ++synced;
            
    resyncPortfolio();
    if (synced != size)
    {         
        // TODO: fetch data from server    
        
    }
}

void StocksMainDialog::addStock(const char_t* name, ulong_t quantity)
{
    StocksPrefs& prefs = GetPreferences()->stocksPrefs;
    StocksPortfolio& p = prefs.portfolio(prefs.currentPortfolio);
    if (!p.addSymbol(name, quantity))
    {
        Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
        return; 
    }
    ulong_t index = p.size() - 1;
    if (!StocksResyncEntry(p.entry(index), prefs.currentPortfolio))
    {
        // TODO: fetch data from server 
    }    
    resyncPortfolio(); 
}
