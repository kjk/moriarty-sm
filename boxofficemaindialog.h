#ifndef BOXOFFICE_MAIN_DIALOG_H__
#define BOXOFFICE_MAIN_DIALOG_H__

#include "ModuleDialog.h"
#include <WindowsCE/Controls.hpp>

struct UniversalDataFormat;

class BoxOfficeMainDialog: public ModuleDialog {

    // ListBox list_;
    ListView listView_; 

    BoxOfficeMainDialog();
    ~BoxOfficeMainDialog();
     
    UniversalDataFormat* udf_;
   
    void fetchData();
    void updateList(); 
    
protected:

    bool handleInitDialog(HWND fw, long ip);
   
    long handleResize(UINT st, ushort w, ushort h);
   
    long handleCommand(ushort nc, ushort id, HWND sender);  
   
    bool handleLookupFinished(Event& event, const LookupFinishedEventData* data); 
   
    bool drawListViewItem(const NMLVCUSTOMDRAW& data); 
   
    bool handleMeasureItem(UINT controlId, MEASUREITEMSTRUCT& data); 

    //long handleNotify(int controlId, const NMHDR& header);  
   
    LRESULT callback(UINT msg, WPARAM wParam, LPARAM lParam);
   
public:
    
    MODULE_DIALOG_CREATE_DECLARE(BoxOfficeMainDialog);

};

#endif // BOXOFFICE_MAIN_DIALOG_H__