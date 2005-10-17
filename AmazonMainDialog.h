#ifndef AMAZON_MAIN_DIALOG_H__
#define AMAZON_MAIN_DIALOG_H__

#include "ModuleDialog.h"
#include <WindowsCE/Controls.hpp>
#include <WindowsCE/WinTextRenderer.hpp>

class HistorySupport;

class AmazonMainDialog: public ModuleDialog {

    HistorySupport* history_;
    TextRenderer renderer_;

    AmazonMainDialog();
    ~AmazonMainDialog();

protected:

    bool handleInitDialog(HWND fw, long ip);
   
    long handleResize(UINT st, ushort w, ushort h);
   
    long handleCommand(ushort nc, ushort id, HWND sender);  
     
    bool handleLookupFinished(Event& event, const LookupFinishedEventData* data); 

public:
    
    MODULE_DIALOG_CREATE_DECLARE(AmazonMainDialog);

};

#endif // AMAZON_MAIN_DIALOG_H__