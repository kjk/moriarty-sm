#ifndef INFOMAN_RECIPES_MAIN_DIALOG_H__
#define INFOMAN_RECIPES_MAIN_DIALOG_H__

#include "ModuleDialog.h"
#include <WindowsCE/Controls.hpp>
#include <WindowsCE/WinTextRenderer.hpp>

class RecipesMainDialog: public ModuleDialog {
    TextRenderer renderer_;
    EditBox term_;
    Widget back_;
    Widget search_;

    RecipesMainDialog();
    ~RecipesMainDialog(); 
    
    DefinitionModel* listModel_;
    DefinitionModel* itemModel_;
         
   
    enum DisplayMode {
        showList,
        showItem,
        showAbout
    } displayMode_;
    char_t* query_; 
   
    void setDisplayMode(DisplayMode dm); 
    void prepareAbout(); 
   
    void search(); 
    
protected:
    
    bool handleInitDialog(HWND focus_widget_handle, long init_param);

    bool handleLookupFinished(Event& event, const LookupFinishedEventData* data);
   
    long handleCommand(ushort notify_code, ushort id, HWND sender);
   
    long handleResize(UINT sizeType, ushort width, ushort height);
    
public:
    
   MODULE_DIALOG_CREATE_DECLARE(RecipesMainDialog);
    
};

#endif // INFOMAN_RECIPES_MAIN_DIALOG_H__