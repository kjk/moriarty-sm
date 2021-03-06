#ifndef MOVIES_MAIN_DIALOG_H__
#define MOVIES_MAIN_DIALOG_H__

#include "ModuleDialog.h"
#include "MoviesData.hpp"

#include <WindowsCE/Controls.hpp>
#include <WindowsCE/WinTextRenderer.hpp>

class MoviesMainDialog: public ModuleDialog {

    UniversalDataFormat* udf_;
    Movies_t movies_;
    
    ListView list_;
    TextRenderer renderer_;

    MoviesMainDialog();
    
    ~MoviesMainDialog();
    
    enum DisplayMode {
        showMovies,
        showTheatres,
        showMovieDetails,
        showTheatreDetails
    } displayMode_;
    
    void setDisplayMode(DisplayMode dm);
    void prepareMoviesList();
    void prepareTheatresList();
    void clearMovies();
    void showMovie(ulong_t index);
    void showTheatre(ulong_t index);

protected:

    bool handleInitDialog(HWND fw, long ip);
   
    long handleResize(UINT st, ushort w, ushort h);
   
    long handleCommand(ushort nc, ushort id, HWND sender);  
     
    bool handleLookupFinished(Event& event, const LookupFinishedEventData* data); 
    
    long handleListItemActivate(int controlId, const NMLISTVIEW& h);

public:
    
    MODULE_DIALOG_CREATE_DECLARE(MoviesMainDialog);


    friend class HyperlinkHandler;
};

#endif // MOVIES_MAIN_DIALOG_H__