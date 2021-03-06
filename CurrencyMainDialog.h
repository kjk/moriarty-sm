#ifndef CURRENCY_MAIN_DIALOG_H__
#define CURRENCY_MAIN_DIALOG_H__

#include "ModuleDialog.h"
#include <WindowsCE/Controls.hpp>

class CurrencyMainDialog: public ModuleDialog {

    Widget label_;
    EditBox edit_;
    ListView list_;
    double amount_;
    double baseRate_;

    CurrencyMainDialog();
    ~CurrencyMainDialog();
    
    void createListColumns();
    void createListItems(bool update = false);
    
    bool handleListItemChanged(NMLISTVIEW& lv);
    void updateAmountField();
    void amountFieldChanged();

protected:

    bool handleInitDialog(HWND fw, long ip);
   
    long handleResize(UINT st, ushort w, ushort h);
   
    long handleCommand(ushort nc, ushort id, HWND sender);  
     
    bool handleLookupFinished(Event& event, const LookupFinishedEventData* data); 
    
    long handleNotify(int controlId, const NMHDR& header);

    bool handleContextMenu(WPARAM wParam, LPARAM lParam); 
    
    LRESULT callback(UINT msg, WPARAM wParam, LPARAM lParam);
    
    long handleDestroy();
    
public:
    
    MODULE_DIALOG_CREATE_DECLARE(CurrencyMainDialog);

};

#endif // CURRENCY_MAIN_DIALOG_H__