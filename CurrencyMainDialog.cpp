#include "CurrencyMainDialog.h"
#include "LookupManager.h"
#include "InfoMan.h"
#include "CurrencyModule.h"
#include "InfoManPreferences.h"

#include <Text.hpp>
#include <Currencies.hpp>
#include <SysUtils.hpp>
#include <UniversalDataHandler.hpp>

using namespace DRA;

class CurrencySelectDialog: public MenuDialog {
    ListView list_;
    
    CurrencySelectDialog():
        MenuDialog(IDR_DONE_CANCEL_MENU)
    {
        setAutoDelete(autoDeleteNot);
    }
    
    ~CurrencySelectDialog()
    {
    }
    
protected:

    void createListColumns();
    void createListItems();
    
    long handleResize(UINT st, ushort width, ushort height)
    {
        list_.anchor(anchorRight, 0, anchorBottom, 0, repaintWidget);
        return messageHandled;
    }

    bool handleInitDialog(HWND fw, long ip)
    {
        list_.attachControl(handle(), IDC_CURRENCY_LIST);
        list_.setStyleEx(LVS_EX_FULLROWSELECT | LVS_EX_GRADIENT | LVS_EX_ONECLICKACTIVATE);
        MenuDialog::handleInitDialog(fw, ip);
        
        createListColumns();
        createListItems();
        
        return true;
    }
    
    long handleCommand(ushort nc, ushort id, HWND sender)
    {
        switch (id) 
        {
            case IDCANCEL:
                endModal(-1);
                return messageHandled;
            case IDOK:
            {
                long sel = list_.selection();
                if (-1 == sel)
                {
                    list_.focus();
                    return messageHandled;
                }
                LVITEM item = {LVIF_PARAM, sel};
                DTEST(FALSE != ListView_GetItem(list_.handle(), &item));
                endModal(item.lParam);
                return messageHandled;
            }
        }

        return MenuDialog::handleCommand(nc, id, sender);
    }
    
    long handleListItemActivate(int controlId, const NMLISTVIEW& header)
    {
        LVITEM item = {LVIF_PARAM, header.iItem};
        DTEST(FALSE != ListView_GetItem(list_.handle(), &item));
        endModal(item.lParam);
        return messageHandled;
    }
    
    
public:

    static long showModal(HWND parent)
    {
        CurrencySelectDialog dlg;
        return dlg.MenuDialog::showModal(NULL, IDD_CURRENCY_SELECT, parent);
    }
    
};

static const struct {
    UINT textId;
    uint_t width;
} currencySelectionColumns[] = {
    {IDS_CURRENCY_SYMBOL, 15},
    {IDS_CURRENCY, 35},
    {IDS_CURRENCY_REGION, 40}
};

void CurrencySelectDialog::createListColumns()
{
    LVCOLUMN col;
    ZeroMemory(&col, sizeof(col));
    col.mask = LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
    
    ulong_t w = list_.width();
    for (ulong_t i = 0; i < ARRAY_SIZE(currencySelectionColumns); ++i)
    {
        char_t* text = LoadString(currencySelectionColumns[i].textId);
        if (NULL == text)
            goto Error; 
        col.iSubItem = i;
        col.iOrder = i;
        col.pszText = text;
        col.cx = (currencySelectionColumns[i].width * w) / 100;
        int res = list_.insertColumn(i, col);
        free(text);
        if (-1 == res)
            goto Error;
    }  
    return; 
Error: 
    Alert(IDS_ALERT_NOT_ENOUGH_MEMORY); 
}

void CurrencySelectDialog::createListItems()
{
    ulong_t size = CurrencyCount();
    LVITEM item;
    ZeroMemory(&item, sizeof(item));
    
    CurrencyPrefs& prefs = GetPreferences()->currencyPrefs;
    long lastItem = 0;
    for (ulong_t i = 0; i < size; ++i)
    {
        if (prefs.isCurrencySelected(i))
            continue;
            
        item.mask = LVIF_TEXT | LVIF_PARAM;
        item.pszText = const_cast<char_t*>(CurrencySymbol(i));
        item.iItem = lastItem++;
        item.iSubItem = 0;
        item.lParam = i;
        list_.insertItem(item);
        item.mask = LVIF_TEXT;
        
        item.pszText = const_cast<char_t*>(CurrencyName(i));
        item.iSubItem++;
        list_.setItem(item);
        
        item.pszText = const_cast<char_t*>(CurrencyRegion(i));
        item.iSubItem++;
        list_.setItem(item);
    }
    list_.focusItem(0);
}

CurrencyMainDialog::CurrencyMainDialog():
    ModuleDialog(IDR_CURRENCY_MENU),
    amount_(1.0),
    baseRate_(1.0)
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
    
    CurrencyPrefs& prefs = GetPreferences()->currencyPrefs;
    if (NULL == prefs.udf)
        prefs.udf = UDF_ReadFromStream(currencyDataStream);

    if (NULL == prefs.udf)
        CurrencyFetchData();
    
    createListItems();
    
    updateAmountField();
    edit_.focus();
    
    return false;
}

void CurrencyMainDialog::updateAmountField()
{
    char_t buffer[32];
    tprintf(buffer, _T("%.2f"), amount_);
    StrNumberApplyGrouping(buffer, 32);
    localizeNumberStrInPlace(buffer);
    edit_.setText(buffer);
}

void CurrencyMainDialog::amountFieldChanged()
{
    char_t* val = edit_.text();
    if (NULL == val)
    {
        Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
        return;
    }
    double a = amount_;
    if (0 == Len(val))
    {
        free(val);
        amount_ = 0;
    }
    else {
        delocalizeNumberStrInPlace(val);
        double v;
        if (errNone != numericValue(val, -1, v))
        {
            free(val);
            Alert(handle(), IDS_INFO_CANT_DELETE_LAST_PORTFOLIO, IDS_INFO, MB_ICONEXCLAMATION | MB_OK);
            updateAmountField();
            return;
        }
        free(val);
        amount_ = v;
    }
    if (amount_ != a)
        createListItems(true);
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
        case IDC_AMOUNT:
            switch (nc)
            {
                case EN_CHANGE:
                    amountFieldChanged();
                    return messageHandled;
            }
            break;
            
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
        
        case ID_VIEW_UPDATE:
        {
            if (errNone != CurrencyFetchData())
                Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
            return messageHandled;
        }
        
        case ID_CURRENCY_ADD:
        {
            long index = CurrencySelectDialog::showModal(handle());
            if (-1 != index)
            {
                if (errNone == prefs.selectCurrency(index))
                    createListItems();
                else
                    Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
            }
            return messageHandled;
        }
    }
    return ModuleDialog::handleCommand(nc, id, sender);
}

bool CurrencyMainDialog::handleLookupFinished(Event& event, const LookupFinishedEventData* data)
{
    switch (data->result)
    {
        case lookupResultCurrency:
        {
            LookupManager* lm = GetLookupManager();
            CurrencyPrefs& prefs = GetPreferences()->currencyPrefs;
            PassOwnership(lm->udf, prefs.udf);
            assert(NULL != prefs.udf);
            ModuleTouchRunning();
            createListItems(true);
            return true;
        }
    }
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


    double amount = amount_;

    ulong_t size = prefs.selectedCurrencies.size();

    if (-1 == sel && 0 != size)
        sel = 0;
    double baseRate = baseRate_;

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
        
    CurrencyPrefs& prefs = GetPreferences()->currencyPrefs;
    ulong_t index = prefs.selectedCurrencies[lv.iItem];
    double newRate = 0.0;
    if (NULL != prefs.udf)
        newRate = GetCurrencyRate(*prefs.udf, index);
    
    if (0 != baseRate_ && 0 != newRate)
    {
        amount_ = amount_ * newRate / baseRate_;
        baseRate_ = newRate;
        updateAmountField();
    }
    else
        baseRate_ = newRate;

    createListItems(true);
    return true;
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
