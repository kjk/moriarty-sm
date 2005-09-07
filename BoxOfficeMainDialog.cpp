#include "BoxOfficeMainDialog.h"
#include "InfoMan.h"
#include "BoxOfficeModule.h"
#include "LookupManager.h"
#include "HyperlinkHandler.h"

#include "MoriartyStyles.hpp"

#include <UniversalDataHandler.hpp>
#include <DefinitionStyle.hpp>
#include <SysUtils.hpp>
#include <Graphics.hpp>
#include <Text.hpp>

using namespace DRA;

BoxOfficeMainDialog::BoxOfficeMainDialog():
    ModuleDialog(IDR_DONE_UPDATE_MENU),
    udf_(NULL) 
{
}

BoxOfficeMainDialog::~BoxOfficeMainDialog()
{
    delete udf_;
} 

MODULE_DIALOG_CREATE_IMPLEMENT(BoxOfficeMainDialog, IDD_BOXOFFICE_MAIN)

bool BoxOfficeMainDialog::handleInitDialog(HWND fw, long ip)
{
    Rect r;
    innerBounds(r);  
    // list_.attachControl(handle(), IDC_LIST);
    // list_.create(LBS_NOSEL | LBS_OWNERDRAWFIXED | WS_TABSTOP | LBS_NOINTEGRALHEIGHT, 0, 0, r.width(), r.height(), handle(), 0);  
    listView_.create(WS_VISIBLE | WS_TABSTOP | LVS_REPORT | LVS_OWNERDRAWFIXED | LVS_NOCOLUMNHEADER | LVS_ALIGNLEFT, 0, 0, r.width(), r.height(), handle(), GetInstance());

#ifndef LVS_EX_GRADIENT
#define LVS_EX_GRADIENT 0
#endif

    listView_.setStyleEx(LVS_EX_GRADIENT | LVS_EX_ONECLICKACTIVATE | LVS_EX_NOHSCROLL | LVS_EX_GRIDLINES);
    
    ModuleDialog::handleInitDialog(fw, ip);
   
    udf_ = UDF_ReadFromStream(boxOfficeDataStream);
    if (NULL == udf_)
        fetchData();
    else
        updateList();       
   
    return true; 
}

void BoxOfficeMainDialog::fetchData()
{
    const char* url = urlSchemaBoxOffice urlSeparatorSchemaStr;
    LookupManager* lm = GetLookupManager();
    status_t err = lm->fetchUrl(url);
    if (errNone != err)
        Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);    
}

long BoxOfficeMainDialog::handleResize(UINT st, ushort w, ushort h)
{
    listView_.anchor(anchorRight, 0, anchorBottom, 0, repaintWidget);
    return ModuleDialog::handleResize(st, w, h);
}

long BoxOfficeMainDialog::handleCommand(ushort nc, ushort id, HWND sender)
{
    switch (id) 
    {
        case ID_VIEW_UPDATE:
            fetchData();
            return messageHandled;
    }  
    return ModuleDialog::handleCommand(nc, id, sender);
}

bool BoxOfficeMainDialog::handleLookupFinished(Event& event, const LookupFinishedEventData* data)
{
    LookupManager* lm = GetLookupManager();
    switch (data->result)
    {
        case lookupResultBoxOfficeData:
            PassOwnership(lm->udf, udf_);
            updateList();
            ModuleTouchRunning();
            return true;
    }
    ModuleDialog::handleLookupFinished(event, data);
    if (NULL == udf_)
        ModuleRunMain(); 
    return true;
}

LRESULT BoxOfficeMainDialog::callback(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) 
    {
        case WM_DRAWITEM:
            assert(false);
            return 0;
        //    return handleDrawItem(wParam, *(const DRAWITEMSTRUCT*)lParam);
        
    }
    return ModuleDialog::callback(msg, wParam, lParam);
}

bool BoxOfficeMainDialog::drawListViewItem(const NMLVCUSTOMDRAW& data)
{
    Rect r;
    ulong_t i = data.nmcd.dwItemSpec;
    ListView_GetItemRect(listView_.handle(), i, &r, LVIR_BOUNDS);
    r.setWidth(listView_.width());
    Graphics g(data.nmcd.hdc, Graphics::deleteNot);
    WinFont font;
    font.getSystemBoldFont();
    g.setFont(font);
       
    char_t buffer[8];
    ulong_t len = tprintf(buffer, TEXT("%ld. "), i + 1);
    
    Point p(r.x() + SCALEX(1) , r.y() + SCALEY(1));
    g.drawText(buffer, len, p);
    p.x += g.textWidth(buffer, len);
    
    const char_t* text = udf_->getItemText(i, boxOfficeTitleIndexInUDF, len);
    g.drawText(text, len, p);
   
    font.getSystemFont();
    g.setFont(font);
     
    p.set(SCALEX(1), r.y() + r.height() / 2);
    text = udf_->getItemText(i, boxOfficeAbbrevGrossIndexInUDF, len);
    g.drawText(text, len, p);
    p.x += g.textWidth(text, len);
   
    text = TEXT(" (week/total)");
    len = Len(text);
    g.drawText(text, len, p); 
    return true;
}

bool BoxOfficeMainDialog::handleMeasureItem(UINT controlId, MEASUREITEMSTRUCT& data)
{  
    Rect r;
    innerBounds(r);  
    data.itemWidth = r.width();

    //const DefinitionStyle* style = StyleGetStaticStyle(styleNameBold);
    //assert(NULL != style);
    //WinFont font = style->font();

    HDC dc = GetDC(NULL);
    assert(NULL != dc);
    HGDIOBJ oldFont = SelectObject(dc, GetStockObject(SYSTEM_FONT));
    TEXTMETRIC m;
    GetTextMetrics(dc, &m);
    SelectObject(dc, oldFont);
    ReleaseDC(NULL, dc);
    
    data.itemHeight = 2 * m.tmHeight + SCALEY(4);
    return true;
}

void BoxOfficeMainDialog::updateList()
{
    assert(NULL != udf_);
    listView_.sendMessage(WM_SETREDRAW, FALSE, 0); 
    listView_.clear();
    ulong_t count = udf_->getItemsCount();
    for (ulong_t i = 0; i < count; ++i)
    {
        const char_t* title = udf_->getItemText(i, boxOfficeTitleIndexInUDF);
        LVITEM item;
        ZeroMemory(&item, sizeof(item));
        item.mask = LVIF_TEXT | LVIF_PARAM;
        item.iItem = i;
        item.iSubItem = 0;
        item.pszText = const_cast<char_t*>(title);
        item.lParam = i;
        
        long res = listView_.insertItem(item);
        if (-1 == res)
            goto Error;
    } 
    listView_.invalidate(erase); 
    listView_.sendMessage(WM_SETREDRAW, TRUE, 0); 
    ListView_RedrawItems(handle(), 0, count - 1);
    return; 
Error:
    listView_.clear(); 
    Alert(IDS_ALERT_NOT_ENOUGH_MEMORY); 
}