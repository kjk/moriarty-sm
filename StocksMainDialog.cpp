#include "StocksMainDialog.h"
#include "LookupManager.h"
#include "InfoMan.h"

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
    list_.attachControl(handle(), IDC_STOCKS_LIST);
    
#ifndef LVS_EX_GRADIENT
#define LVS_EX_GRADIENT 0
#endif

    list_.setStyleEx(LVS_EX_GRADIENT);
    list_.setTextBkColor(CLR_NONE);
    
    ModuleDialog::handleInitDialog(fw, ip);
   
    return false; 
}

long StocksMainDialog::handleResize(UINT st, ushort w, ushort h)
{
    list_.anchor(anchorRight, 0, anchorBottom, 0, repaintWidget);
    return ModuleDialog::handleResize(st, w, h);
}

long StocksMainDialog::handleCommand(ushort nc, ushort id, HWND sender)
{
    return ModuleDialog::handleCommand(nc, id, sender);
}

bool StocksMainDialog::handleLookupFinished(Event& event, const LookupFinishedEventData* data)
{
    switch (data->result)
    {
    }
    return ModuleDialog::handleLookupFinished(event, data);
}
