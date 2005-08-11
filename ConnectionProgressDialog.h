#ifndef INFOMAN_CONNECTION_PROGRESS_DIALOG_H__
#define INFOMAN_CONNECTION_PROGRESS_DIALOG_H__

#include <WindowsCE/Dialog.hpp>

class ConnectionProgressDialog: public Dialog {
    Widget progressBar_;
    
    ConnectionProgressDialog(AutoDeleteOption ad);

    static long showModal(HWND owner); 
    
public:

    static ConnectionProgressDialog* create(HWND parent);
    
protected:

    bool handleInitDialog(HWND focus_widget_handle, long init_param);
   
	long handleCommand(ushort notify_code, ushort id, HWND sender);

	long handleResize(UINT sizeType, ushort width, ushort height);
	
};

#endif // INFOMAN_CONNECTION_PROGRESS_DIALOG_H__