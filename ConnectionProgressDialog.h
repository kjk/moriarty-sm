#ifndef INFOMAN_CONNECTION_PROGRESS_DIALOG_H__
#define INFOMAN_CONNECTION_PROGRESS_DIALOG_H__

#include <WindowsCE/Dialog.hpp>
#include <WindowsCE/Controls.hpp>
#include <ExtendedEvent.hpp>

class LookupManager;
class InfoManConnection;

class ConnectionProgressDialog: public Dialog {
    ExtEventHelper extEventHelper_;
    LookupManager& lookupManager_; 
    ProgressBar progressBar_;
    Widget progressBytesText_; 
    InfoManConnection* connectionToEnqueue_; 
    
    ConnectionProgressDialog(AutoDeleteOption ad, InfoManConnection* conn);

   
    void updateProgress(); 
    
    static ConnectionProgressDialog* create(HWND parent, InfoManConnection* conn);
    
public:

    static status_t showModal(HWND owner, InfoManConnection* conn); 
    
protected:

    bool handleInitDialog(HWND focus_widget_handle, long init_param);
   
    long handleCommand(ushort notify_code, ushort id, HWND sender);

    long handleResize(UINT sizeType, ushort width, ushort height);

    long handleExtendedEvent(LPARAM& event);

    void handleScreenSizeChange(ulong_t w, ulong_t h);

};

#endif // INFOMAN_CONNECTION_PROGRESS_DIALOG_H__