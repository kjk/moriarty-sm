#include "StocksMainDialog.h"
#include "LookupManager.h"
#include "InfoMan.h"
#include "StocksModule.h"
#include "InfoManPreferences.h"
#include "MoriartyStyles.hpp"
#include "HyperlinkHandler.h"

#include <SysUtils.hpp>
#include <Text.hpp>
#include <Utility.hpp>
#include <DefinitionStyle.hpp>
#include <WindowsCE/WinTextRenderer.hpp>
#include <UniversalDataFormat.hpp>

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
            {
                char_t* cap = edit_.caption();
                free(name_); 
                name_ = StocksValidatePortfolioName(cap);
                free(cap); 
                if (NULL == name_)
                {
                    edit_.focus();
                    edit_.setSelection();
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
#ifdef SHELL_SIP            
        SHSipPreference(parent, SIP_FORCEDOWN);
#endif            
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
                    char_t* cap = symbolBox_.caption();
                    free(symbol_);
                    symbol_ = StocksValidateTicker(cap);
                    free(cap);
                    if (NULL == symbol_)
                    {
                        symbolBox_.setSelection();
                        symbolBox_.focus();
                        return messageHandled;
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
#ifdef SHELL_SIP            
        SHSipPreference(parent, SIP_FORCEDOWN);
#endif            
        return res;
    }  
      
};

class StocksDetailsDialog: public MenuDialog {
    TextRenderer renderer_; 
    const StocksEntry& entry_; 
    ExtEventHelper extHelper_; 
    
    StocksDetailsDialog(DefinitionModel* model, const StocksEntry& entry):
        MenuDialog(IDR_DONE_UPDATE_MENU),
        entry_(entry)
    {
        setInitFlags(SHIDIF_DONEBUTTON | SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_SIPDOWN);
        setMenuBarFlags(SHCMBF_HIDESIPBUTTON);
        setAutoDelete(autoDeleteNot);
        renderer_.setModel(model, Definition::ownModel); 
    }   
    
    ~StocksDetailsDialog()
    {
    }
protected:
    
    bool handleInitDialog(HWND fw, long ip)
    {
        Rect r;
        innerBounds(r); 
        renderer_.create(WS_VISIBLE | WS_TABSTOP, 0, 0, r.width(), r.height(), handle());
        renderer_.definition.setHyperlinkHandler(GetHyperlinkHandler());
        renderer_.definition.setInteractionBehavior(Definition::behavHyperlinkNavigation | Definition::behavMouseSelection | Definition::behavUpDownScroll);
        setCaption(IDS_STOCKS_DETAILS);
        extHelper_.start(handle());
        
        MenuDialog::handleInitDialog(fw, ip);
        
        renderer_.focus();
        return false;
    }   
   
    long handleResize(UINT, ushort, ushort)
    {
        renderer_.anchor(anchorRight, 0, anchorBottom, 0, repaintWidget);
        return messageHandled; 
    }   
   
    long handleCommand(ushort nc, ushort id, HWND sender)
    {
        switch (id)
        {
            case IDCANCEL:
            case IDOK:
                extHelper_.stop();
                endModal(id);
                return messageHandled;
            case ID_VIEW_UPDATE:
                if (errNone != StocksFetchDetails(entry_.url, (entry_.statusReady == entry_.status || entry_.statusUnknown == entry_.status)))
                    Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
                return messageHandled;  
        }
        return MenuDialog::handleCommand(nc, id, sender);  
    }   
   
    long handleExtendedEvent(Event& event)
    {
        switch (ExtEventGetID(event))
        {
            case extEventLookupFinished:
                extHelper_.stop();
                endModal(IDCANCEL);
                ExtEventRepost(event);  
                break; 
        }
        return MenuDialog::handleExtendedEvent(event); 
    }  
     
public:
          
    static long showModal(DefinitionModel* model, const StocksEntry& entry, HWND parent) 
    {
        StocksDetailsDialog dlg(model, entry);
        return dlg.MenuDialog::showModal(GetInstance(), IDD_EMPTY, parent);
    }
    
}; 

class StocksValidateTickerDialog: public MenuDialog {

    UniversalDataFormat* udf_;
    ListView list_;
    char_t* symbol_; 
   
    StocksValidateTickerDialog(UniversalDataFormat* udf):
        MenuDialog(IDR_DONE_CANCEL_MENU),
        udf_(udf),
        symbol_(NULL)
    {
        setAutoDelete(autoDeleteNot); 
        setInitFlags(SHIDIF_DONEBUTTON | SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_SIPDOWN);
        setMenuBarFlags(SHCMBF_HIDESIPBUTTON);
    }
   
    ~StocksValidateTickerDialog()
    {
        delete udf_;
        free(symbol_);
    }    
   
   void createColumns();
   void createListItems();
   
protected:

    long handleResize(UINT, ushort, ushort)
    {
        list_.anchor(anchorRight, 0, anchorBottom, 0, repaintWidget);
        return messageHandled; 
    }   

    bool handleInitDialog(HWND fw, long ip)
    {
        Rect r;
        innerBounds(r); 
        list_.create(WS_VISIBLE | WS_TABSTOP | LVS_ALIGNLEFT | LVS_REPORT | LVS_REPORT | LVS_SINGLESEL, 0, 0, r.width(), r.height(), handle(), NULL);
        list_.setStyleEx(LVS_EX_FULLROWSELECT | LVS_EX_GRADIENT);
        createColumns();
        createListItems();
        setCaption(IDS_STOCKS_SELECT_TICKER);
                
        MenuDialog::handleInitDialog(fw, ip);
        
        list_.focus();
        return false;
    }   
   
    long handleListItemActivate(int controlId, const NMLISTVIEW& header)
    {
        symbol_ = StringCopy(udf_->getItemText(header.iItem, 1));
        endModal(IDOK);
        return messageHandled;
    }   
   
    long handleCommand(ushort nc, ushort id, HWND sender)
    {
        switch (id) 
        {
            case IDOK:
            {
                long sel = list_.selection();
                if (-1 == sel)
                    return messageHandled;
                symbol_ = StringCopy(udf_->getItemText(sel, 1));
            }
            // intentional fall-through
            case IDCANCEL:
                endModal(id);
                return messageHandled;
        }    
        return MenuDialog::handleCommand(nc, id, sender);
    }   

public:

    static long showModal(UniversalDataFormat* udf, char_t*& symbol, HWND parent)
    {
        StocksValidateTickerDialog dlg(udf);
        long res = dlg.MenuDialog::showModal(GetInstance(), IDD_EMPTY, parent);
        if (IDOK == res)
        {
            free(symbol);
            symbol = dlg.symbol_;
            dlg.symbol_ = NULL;
        }
        return res;
    }

}; 

static const UINT validateColumnNameIds[] = {
    IDS_STOCK_SYMBOL,
    IDS_NAME,
    IDS_MARKET,
    IDS_INDUSTRY
};    

void StocksValidateTickerDialog::createColumns()
{
    LVCOLUMN col;
    ZeroMemory(&col, sizeof(col));
    col.mask = LVCF_ORDER | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
    ulong_t w = list_.width() / ARRAY_SIZE(validateColumnNameIds);
    for (ulong_t i = 0; i < ARRAY_SIZE(validateColumnNameIds); ++i)
    {
        char_t* text = LoadString(validateColumnNameIds[i]);
        if (NULL == text)
            goto Error; 
        col.iSubItem = i;
        col.iOrder = i;
        col.pszText = text;
        col.cx = w;
        int res = list_.insertColumn(i, col);
        free(text);
        if (-1 == res)
            goto Error;
    }  
    return; 
Error: 
    Alert(IDS_ALERT_NOT_ENOUGH_MEMORY); 
}

void StocksValidateTickerDialog::createListItems()
{
    list_.clear();
    
    LVITEM item;
    ZeroMemory(&item, sizeof(item));
    item.mask = LVIF_TEXT;
    
    ulong_t size = udf_->getItemsCount();
    for (ulong_t i = 0; i < size; ++i)
    {
        item.iItem = i;
        ulong_t ss = udf_->getItemElementsCount(i);
        for (ulong_t j = 1; j < ss; ++j)
        {
            item.iSubItem = j - 1;
            item.pszText = const_cast<char_t*>(udf_->getItemText(i, j));
            long res;
            if (1 == j)
                res = list_.insertItem(item);
            else
                res = ListView_SetItem(list_.handle(), &item); 
        }
    }
}

StocksMainDialog::StocksMainDialog():
    ModuleDialog(IDR_STOCKS_MENU)
{
    setMenuBarFlags(SHCMBF_HIDESIPBUTTON);
    setInitFlags(SHIDIF_DONEBUTTON | SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_SIPDOWN);
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
    StocksPortfolio& p = prefs.current();
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
            long index = list_.selection();
            if (-1 == index)
                return messageHandled;
            assert(ulong_t(index) < p.size());
            if (StrStartsWith(p.entry(index).symbol, _T("^")))
            {
                Alert(handle(), IDS_INFO_CUMULATIVE_INDEX, IDS_INFO, MB_OK | MB_ICONINFORMATION);
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
            long index = list_.selection();
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
            if (IDOK ==  StocksEditPortfolioDialog::showModal(name, handle(), -1) && NULL != name)
                createPortfolio(name);
            free(name);
            return messageHandled;
       
        case ID_VIEW_UPDATE:
            if (errNone != StocksUpdate())
                Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
            return messageHandled; 
        
        case ID_STOCK_DETAILS:
        {
            long item = list_.selection();
            if (-1 == item)
                return messageHandled;
             
            const StocksEntry& e = p.entry(item); 
            const char* url = e.url;
            if (NULL != url)
            {
                status_t err = StocksFetchDetails(url, (e.statusReady == e.status || e.statusUnknown == e.status));
                if (errNone != err)
                    Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
            }
            else ;
                // TODO: show alert that list must be updated first??
            return messageHandled; 
        }  
    }
    return ModuleDialog::handleCommand(nc, id, sender);
}

bool StocksMainDialog::handleLookupFinished(Event& event, const LookupFinishedEventData* data)
{
    LookupManager* lm = GetLookupManager();
    UniversalDataFormat* udf = NULL; 
    switch (data->result)
    {
        case lookupResultStocksList:
        {
            PassOwnership(lm->udf, udf);
            assert(NULL != udf);
            bool res = StocksUpdateFromUDF(*udf);
            delete udf;
            if (res)
            {
                resyncPortfolio();
                ModuleTouchRunning();
            }  
            else
                Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
            return true;
        }  
        
        case lookupResultStock:
        {
            PassOwnership(lm->udf, udf);
            assert(NULL != udf);
            DefinitionModel* model = StocksDetailsFromUDF(*udf);
            delete udf;
            if (NULL == model)
            {
                Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
                return true; 
            }
            // TODO: change this snippet to reference the real entry used when FetchDetails was called
            long l = list_.selection();
            if (-1 == l) l = 0;
            
            StocksDetailsDialog::showModal(model, GetPreferences()->stocksPrefs.current().entry(l), handle());
            return true;
        }
        
        case lookupResultStocksListByName:
            PassOwnership(lm->udf, udf);
            assert(NULL != udf);
            validateLastStock(udf);
            return true;

    }
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
    
    if (1 == size)
        portfolioCombo_.setEnabled(false);  
}

void StocksMainDialog::resyncPortfolio()
{
    list_.clear();
    const StocksPortfolio& p = GetPreferences()->stocksPrefs.current();
    ulong_t size = p.size();
   
    LVITEM item;
    ZeroMemory(&item, sizeof(item));
    item.mask = LVIF_TEXT;
    
    char_t buffer[64];
    double value = 0; 
    
    for (ulong_t i = 0; i < size; ++i)
    {
        const StocksEntry& e = p.entry(i);
                
        if (e.statusReady == e.status && p.valueNotAvailable != e.trade)
            value += e.quantity * e.trade;

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
                    if (e.statusReady == e.status || e.statusUnknown == e.status)
                    { 
                        if (!StrStartsWith(e.symbol, _T("^")))
                        { 
                            tprintf(buffer, _T("%ld"), e.quantity);
                            StrNumberApplyGrouping(buffer, 64);
                            localizeNumberStrInPlace(buffer);
                            item.pszText = buffer;
                        }
                        else
                            item.pszText = _T("");
                    }
                    else if (e.statusChanged == e.status)
                    {
                        tprintf(buffer, _T("Look up symbol for: %s"), e.data);
                        item.pszText = buffer; 
                    }
                    else if (e.statusAmbiguous == e.status)
                    {
                        item.pszText = e.data;
                    }
                    break;
                case scTrade:
                    if (e.statusReady != e.status && e.statusUnknown != e.status)
                        continue;

                    if (StocksPortfolio::valueNotAvailable != e.trade)
                    {
                        tprintf(buffer, _T("%.2f"), e.trade);
                        StrNumberApplyGrouping(buffer, 64);
                        localizeNumberStrInPlace(buffer);
                        item.pszText = buffer;
                    }
                    else
                        item.pszText = _T("N/A");
                    break;
                // TODO: make these owner draw to set text color properly                    
                case scChange:
                    if (e.statusReady != e.status && e.statusUnknown != e.status)
                        continue;

                    if (StocksPortfolio::valueNotAvailable != e.change)
                    {
                        tprintf(buffer, _T("%.2f"), e.change);
                        StrNumberApplyGrouping(buffer, 64);
                        localizeNumberStrInPlace(buffer);
                        item.pszText = buffer;
                    }
                    else
                        item.pszText = _T("N/A");
                    break;
                case scPercentChange:
                    if (e.statusReady != e.status && e.statusUnknown != e.status)
                        continue;

                    if (StocksPortfolio::valueNotAvailable != e.percentChange)
                    {
                        if (e.percentChange < 0.01 && e.percentChange > -0.01)
                            tprintf(buffer, _T("%.2f%%"), e.percentChange);
                        else 
                            tprintf(buffer, _T("%+.2f%%"), e.percentChange);
                        StrNumberApplyGrouping(buffer, 64);
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
    tprintf(buffer, _T("%.2f"), value);
    StrNumberApplyGrouping(buffer, 64);
    localizeNumberStrInPlace(buffer); 
    portfolioValue_.setCaption(buffer); 
}

bool StocksMainDialog::drawListViewItem(NMLVCUSTOMDRAW& data)
{
    const StocksPortfolio& p = GetPreferences()->stocksPrefs.current();
    const StocksEntry& e = p.entry(data.nmcd.dwItemSpec);
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
            
    if (synced != size && errNone != StocksUpdate())
        Alert(IDS_ALERT_NOT_ENOUGH_MEMORY); 
    resyncPortfolio();
}

void StocksMainDialog::addStock(const char_t* name, ulong_t quantity)
{
    StocksPrefs& prefs = GetPreferences()->stocksPrefs;
    StocksPortfolio& p = prefs.current();
    if (!p.addSymbol(name, quantity))
    {
        Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
        return; 
    }
    ulong_t index = p.size() - 1;
    if (!StocksResyncEntry(p.entry(index), prefs.currentPortfolio))
    {
        status_t err = StocksUpdate(true);
        if (errNone != err)
            Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
    }    
    resyncPortfolio(); 
}

long StocksMainDialog::handleListItemActivate(int controlId, const NMLISTVIEW& h)
{
    ulong_t item = h.iItem;
    const StocksPortfolio& p = GetPreferences()->stocksPrefs.current();
    const StocksEntry& e = p.entry(item); 
    const char* url =  e.url;
    if (NULL == url)
        return messageHandled;
          
    status_t err = StocksFetchDetails(url, (e.statusReady == e.status || e.statusUnknown == e.status));
    if (errNone != err)
        Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
    return messageHandled;
}

LRESULT StocksMainDialog::callback(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_CONTEXTMENU:
            if (handleContextMenu(wParam, lParam))
                return messageHandled;
    }
    return ModuleDialog::callback(msg, wParam, lParam);   
}

bool StocksMainDialog::handleContextMenu(WPARAM wParam, LPARAM lParam)
{
    if (list_.handle() != (HWND)wParam)
        return false;
    
    HMENU menu = menuBar().subMenu(IDM_STOCK); 
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
            ListView_SetItemState(list_.handle(), ht.iItem, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
    }    
    
     
    if (onlyAdd)
    {
        EnableMenuItem(menu, ID_STOCK_DETAILS, MF_GRAYED); 
        EnableMenuItem(menu, ID_STOCK_DELETE, MF_GRAYED);
        EnableMenuItem(menu, ID_STOCK_SET_QUANTITY, MF_GRAYED);
    }   
     
    TrackPopupMenu(menu, TPM_TOPALIGN | TPM_LEFTALIGN, p.x, p.y, 0, handle(), NULL);
   
    if (onlyAdd)
    {
        EnableMenuItem(menu, ID_STOCK_DETAILS, MF_ENABLED); 
        EnableMenuItem(menu, ID_STOCK_DELETE, MF_ENABLED);
        EnableMenuItem(menu, ID_STOCK_SET_QUANTITY, MF_ENABLED);
    }           
    
    return true;
}

void StocksMainDialog::validateLastStock(UniversalDataFormat* udf)
{
    StocksPortfolio& p = GetPreferences()->stocksPrefs.current();
     
    char_t* symbol = NULL;
    long res = StocksValidateTickerDialog::showModal(udf, symbol, handle());
    
    long q = p.entry(p.size() - 1).quantity;
    p.removeSymbol(p.size() - 1);
    if (IDOK == res)
    {
        bool r = p.addSymbol(symbol, q);
        resyncPortfolio();
        if (!r)
            Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
        else
            StocksUpdate();
    }
    else
        resyncPortfolio();
    free(symbol);
}