#include "BoxOfficeMainDialog.h"
#include "InfoMan.h"

#include <SysUtils.hpp>

BoxOfficeMainDialog::BoxOfficeMainDialog():
    ModuleDialog(IDR_BOXOFFICE_MENU)
{
}

BoxOfficeMainDialog::~BoxOfficeMainDialog()
{
} 

MODULE_DIALOG_CREATE_IMPLEMENT(BoxOfficeMainDialog, IDD_BOXOFFICE_MAIN)

bool BoxOfficeMainDialog::handleInitDialog(HWND fw, long ip)
{
    list_.attachControl(handle(), IDC_LIST);
    
    ModuleDialog::handleInitDialog(fw, ip);
   
    return true; 
}

long BoxOfficeMainDialog::handleResize(UINT st, ushort w, ushort h)
{
    list_.anchor(anchorRight, 2 * LogX(1), anchorBottom, 2 * LogY(1), repaintWidget);
    return ModuleDialog::handleResize(st, w, h);
}

long BoxOfficeMainDialog::handleCommand(ushort nc, ushort id, HWND sender)
{
    return ModuleDialog::handleCommand(nc, id, sender);
}
