#ifndef INFOMAN_CONNECTION_PROGRESS_DIALOG_H__
#define INFOMAN_CONNECTION_PROGRESS_DIALOG_H__

#include <WindowsCE/Dialog.hpp>
#include <WindowsCE/Controls.hpp>
#include <ExtendedEvent.hpp>

class LookupManager;

class ConnectionProgressDialog: public Dialog {
    ExtEventHelper extEventHelper_;
    LookupManager& lookupManager_; 
    ProgressBar progressBar_;
    Widget progressBytesText_; 
    
    ConnectionProgressDialog(AutoDeleteOption ad);

   
    void updateProgress(); 
    
public:

    static ConnectionProgressDialog* create(HWND parent);
    static long showModal(HWND owner); 
    
protected:

    bool handleInitDialog(HWND focus_widget_handle, long init_param);
   
	long handleCommand(ushort notify_code, ushort id, HWND sender);

	long handleResize(UINT sizeType, ushort width, ushort height);
	
	long handleExtendedEvent(LPARAM& event);
	    
	void handleScreenSizeChange(ulong_t w, ulong_t h);
	
};

#endif // INFOMAN_CONNECTION_PROGRESS_DIALOG_H__