#ifndef STOCKS_MAIN_DIALOG_H__
#define STOCKS_MAIN_DIALOG_H__

#include "ModuleDialog.h"
#include <WindowsCE/Controls.hpp>

class StocksMainDialog: public ModuleDialog {

    ListView list_;

    StocksMainDialog();
    ~StocksMainDialog();

protected:

    bool handleInitDialog(HWND fw, long ip);
   
    long handleResize(UINT st, ushort w, ushort h);
   
    long handleCommand(ushort nc, ushort id, HWND sender);  
     
    bool handleLookupFinished(Event& event, const LookupFinishedEventData* data); 

public:
    
    MODULE_DIALOG_CREATE_DECLARE(StocksMainDialog);

};

#endif // STOCKS_MAIN_DIALOG_H__