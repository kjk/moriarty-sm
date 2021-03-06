#ifndef STOCKS_MAIN_DIALOG_H__
#define STOCKS_MAIN_DIALOG_H__

#include "ModuleDialog.h"
#include <WindowsCE/Controls.hpp>

struct UniversalDataFormat;

class StocksMainDialog: public ModuleDialog {
    
    ComboBox portfolioCombo_; 
    ListView list_;
    EditBox portfolioValue_; 

    StocksMainDialog();
    ~StocksMainDialog();
    
    void resyncPortfoliosCombo(); 
    void resyncPortfolio(); 
    void createColumns();
    void createPortfolio(const char_t* name); 
    void addStock(const char_t* name, ulong_t quantity); 
    void validateLastStock(UniversalDataFormat* udf); 
    
protected:

    bool handleInitDialog(HWND fw, long ip);
   
    long handleResize(UINT st, ushort w, ushort h);
   
    long handleCommand(ushort nc, ushort id, HWND sender);  
     
    bool handleLookupFinished(Event& event, const LookupFinishedEventData* data); 

    bool drawListViewItem(NMLVCUSTOMDRAW& data);
   
    bool handleContextMenu(WPARAM wParam, LPARAM lParam); 
    
    LRESULT callback(UINT msg, WPARAM wParam, LPARAM lParam);  
    
    long handleListItemActivate(int controlId, const NMLISTVIEW& h);
    
public:
    
    MODULE_DIALOG_CREATE_DECLARE(StocksMainDialog);

};

#endif // STOCKS_MAIN_DIALOG_H__