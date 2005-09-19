#ifndef CHANGE_LOCATION_DIALOG_H__
#define CHANGE_LOCATION_DIALOG_H__

#include "ModuleDialog.h"
#include <WindowsCE/Controls.hpp>

class ChangeLocationDialog: public MenuDialog {

    char_t* location_;
    EditBox edit_;
   
    ChangeLocationDialog(char_t* location);
    ~ChangeLocationDialog();

protected:

    bool handleInitDialog(HWND fw, long ip);
    long handleCommand(ushort nc, ushort id, HWND sender);     

public:

    static long showModal(char_t*& location, HWND parent);

};

#endif // CHANGE_LOCATION_DIALOG_H__