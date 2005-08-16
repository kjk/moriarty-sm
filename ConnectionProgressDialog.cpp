#include <SysUtils.hpp>
#include <ExtendedEvent.hpp>
#include <Text.hpp>
#include "ConnectionProgressDialog.h"
#include "InfoMan.h"
#include "InfoManGlobals.h"
#include "LookupManager.h"

using namespace DRA;

ConnectionProgressDialog::ConnectionProgressDialog(AutoDeleteOption ad):
    Dialog(ad, false, SHIDIF_SIPDOWN),
    lookupManager_(*GetLookupManager()) 
{
    setSizeToInputPanel(false);
} 

bool ConnectionProgressDialog::handleInitDialog(HWND focus_widget_handle, long init_param)
{
    extEventHelper_.start(handle());
    Dialog::handleInitDialog(focus_widget_handle, init_param);
    handleScreenSizeChange(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)); 
    progressBar_.attachControl(handle(), IDC_CONNECTION_PROGRESS);
    progressBytesText_.attachControl(handle(), IDC_PROGRESS_BYTES); 
    progressBar_.setRange(0, 100); 
    updateProgress(); 
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
        {
            bool active = lookupManager_.connectionManager().active();
            lookupManager_.connectionManager().abortConnections();
            destroy();
            if (!active)
                return messageHandled;
                
            LookupFinishedEventData* data = new_nt LookupFinishedEventData();
            if (NULL != data)
            {
                data->result = lookupResultConnectionCancelledByUser;
                ExtEventSendObject(extEventLookupFinished, data);
            } 
            return messageHandled;
        }
    }
    return Dialog::handleCommand(notify_code, id, sender);  
}

long ConnectionProgressDialog::handleResize(UINT sizeType, ushort width, ushort height)
{
    anchorChild(IDC_PROGRESS_TEXT, anchorRight, SCALEX(20), anchorNone, 0);
    progressBar_.anchor(anchorRight, SCALEX(20), anchorNone, 0);
    progressBytesText_.anchor(anchorRight, SCALEX(20), anchorNone, 0);
    anchorChild(IDCANCEL, anchorLeft, SCALEX(86), anchorNone, 0);  
    return Dialog::handleResize(sizeType, width, height);
}

long ConnectionProgressDialog::handleExtendedEvent(LPARAM& event)
{
    switch (ExtEventGetID(event))
    {
        case extEventLookupStarted:
        case extEventLookupProgress:
            updateProgress();
            return messageHandled;
            
        case extEventLookupFinished:
        {
            const LookupFinishedEventData* data = LookupFinishedData(event);
            bool handled = lookupManager_.handleLookupFinishedInForm(event);
            destroy();
            if (!handled)
                ExtEventRepost(event);
            return messageHandled;
        }
    }     
    return Dialog::handleExtendedEvent(event); 
}

void ConnectionProgressDialog::updateProgress()
{
    char_t buffer[32];
    uint_t percent = lookupManager_.percentProgress();
    if (LookupManager::percentProgressDisabled == percent)
    { 
        progressBar_.hide();
        // TODO: add pretty formatting of bytes
        if (0 == lookupManager_.bytesProgress())
            progressBytesText_.hide();
        else
        {
            tprintf(buffer, TEXT("Downloading: %ul bytes."), lookupManager_.bytesProgress());
            progressBytesText_.setCaption(buffer);
            progressBytesText_.show();
        }
    }
    else
    {
        progressBytesText_.hide(); 
        progressBar_.setPosition(percent);
        progressBar_.show();
    }
    if (NULL == lookupManager_.statusText || 0 == Len(lookupManager_ .statusText))
        SetDlgItemText(handle(), IDC_PROGRESS_TEXT, TEXT("Please wait..."));
    else
        SetDlgItemText(handle(), IDC_PROGRESS_TEXT, lookupManager_.statusText);
}

void ConnectionProgressDialog::handleScreenSizeChange(ulong_t w, ulong_t h)
{
    Rect r;
    bounds(r);
   
    if (r.width() + SCALEX(4) != w)
    {
        r.x() = SCALEX(2);
        r.setWidth(w - SCALEX(4));
        setBounds(r, repaintWidget); 
    }    
}
