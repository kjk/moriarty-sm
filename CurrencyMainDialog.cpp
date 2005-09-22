#include "CurrencyMainDialog.h"
#include "LookupManager.h"
#include "InfoMan.h"

CurrencyMainDialog::CurrencyMainDialog():
    ModuleDialog(IDR_CURRENCY_MENU)
{
}

CurrencyMainDialog::~CurrencyMainDialog()
{
}

MODULE_DIALOG_CREATE_IMPLEMENT(CurrencyMainDialog, IDD_EMPTY)

bool CurrencyMainDialog::handleInitDialog(HWND fw, long ip)
{
    return ModuleDialog::handleInitDialog(fw, ip);
}

long CurrencyMainDialog::handleResize(UINT st, ushort w, ushort h)
{
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
