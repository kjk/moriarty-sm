#include <SysUtils.hpp>
#include "ConnectionProgressDialog.h"
#include "InfoMan.h"

ConnectionProgressDialog::ConnectionProgressDialog(AutoDeleteOption ad):
    Dialog(ad, false, SHIDIF_SIPDOWN)
{
} 

bool ConnectionProgressDialog::handleInitDialog(HWND focus_widget_handle, long init_param)
{
    Dialog::handleInitDialog(focus_widget_handle, init_param);
    progressBar_.attachControl(handle(), IDC_CONNECTION_PROGRESS);
    return true;  
}

ConnectionProgressDialog* ConnectionProgressDialog::create(HWND parent)
{
    ConnectionProgressDialog* dlg = new_nt ConnectionProgressDialog(autoDelete);
    if (NULL == dlg)
        return NULL;

    bool res = dlg->Dialog::create(GetInstance(), IDD_CONNECTION_PROGRESS, parent);
    if (!res)
    { 
        delete dlg;
        return NULL;
    }
    return dlg; 
}

long ConnectionProgressDialog::showModal(HWND owner)
{
    ConnectionProgressDialog dlg(autoDeleteNot);
    return dlg.Dialog::showModal(GetInstance(), IDD_CONNECTION_PROGRESS, owner);
}


long ConnectionProgressDialog::handleCommand(ushort notify_code, ushort id, HWND sender)
{
    switch (id) {
        case IDCANCEL:
            destroy();
            return messageHandled;
    }
    return Dialog::handleCommand(notify_code, id, sender);  
}

long ConnectionProgressDialog::handleResize(UINT sizeType, ushort width, ushort height)
{
    return Dialog::handleResize(sizeType, width, height);
}


