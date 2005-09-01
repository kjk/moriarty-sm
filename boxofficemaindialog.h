#ifndef BOXOFFICE_MAIN_DIALOG_H__
#define BOXOFFICE_MAIN_DIALOG_H__

#include "ModuleDialog.h"
#include <WindowsCE/Controls.hpp>

class BoxOfficeMainDialog: public ModuleDialog {

    ListBox list_;

    BoxOfficeMainDialog();
    ~BoxOfficeMainDialog();

protected:

    bool handleInitDialog(HWND fw, long ip);
   
    long handleResize(UINT st, ushort w, ushort h);
   
    long handleCommand(ushort nc, ushort id, HWND sender);  
     
public:
    
    MODULE_DIALOG_CREATE_DECLARE(BoxOfficeMainDialog);

};

#endif // BOXOFFICE_MAIN_DIALOG_H__