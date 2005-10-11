#include "MoviesMainDialog.h"
#include "LookupManager.h"
#include "InfoMan.h"
#include "InfoManPreferences.h"
#include "ChangeLocationDialog.h"
#include "MoviesModule.h"

#include <UniversalDataHandler.hpp>
#include <Text.hpp>
#include <SysUtils.hpp>

MoviesMainDialog::MoviesMainDialog():
    ModuleDialog(IDR_MOVIES_MENU),
    udf_(NULL),
    displayMode_(showMovies)
{
}

MoviesMainDialog::~MoviesMainDialog()
{
    delete udf_;
    clearMovies();
}

void MoviesMainDialog::clearMovies()
{
    for (ulong_t i = 0; i < movies_.size(); ++i)
        delete movies_[i];
    movies_.clear();
}

MODULE_DIALOG_CREATE_IMPLEMENT(MoviesMainDialog, IDD_MOVIES_MAIN)

bool MoviesMainDialog::handleInitDialog(HWND fw, long ip)
{
    list_.attachControl(handle(), IDC_LIST);
    list_.setStyleEx(LVS_EX_GRADIENT | LVS_EX_FULLROWSELECT | LVS_EX_ONECLICKACTIVATE);
    
    Rect r;
    innerBounds(r);
    renderer_.create(WS_TABSTOP, r, handle());

    ModuleDialog::handleInitDialog(fw, ip);
    
    setDisplayMode(displayMode_);
    
    UniversalDataFormat* udf = UDF_ReadFromStream(moviesDataStream);
    if (NULL != udf)
    {
        udf_ = udf;
        setDisplayMode(showTheatres);
    }
    
    Preferences& prefs = *GetPreferences();
    if (NULL == prefs.moviesLocation || 0 == Len(prefs.moviesLocation))
    {
        char_t* loc = NULL;
        if (IDOK == ChangeLocationDialog::showModal(loc, handle()))
        {
            free(prefs.moviesLocation);
            prefs.moviesLocation = loc;
            if (NULL != prefs.moviesLocation)
                MoviesFetchData();
        }
    }
        
    return false;
}

long MoviesMainDialog::handleResize(UINT st, ushort w, ushort h)
{
    list_.anchor(anchorRight, 0, anchorBottom, 0, repaintWidget);
    renderer_.anchor(anchorRight, 0, anchorBottom, 0, repaintWidget);
    return ModuleDialog::handleResize(st, w, h);
}

long MoviesMainDialog::handleCommand(ushort nc, ushort id, HWND sender)
{
    return ModuleDialog::handleCommand(nc, id, sender);
}

bool MoviesMainDialog::handleLookupFinished(Event& event, const LookupFinishedEventData* data)
{
    LookupManager* lm = GetLookupManager();
    Preferences& prefs = *GetPreferences();
    switch (data->result)
    {
    case lookupResultMoviesData:
    {
        PassOwnership(lm->udf, udf_);
        assert(NULL != udf_);
        clearMovies();
        prepareTheatresList();
        setDisplayMode(showTheatres);
        return messageHandled;
    }
        
    case lookupResultLocationUnknown:
    {
        char_t* loc = StringCopy(prefs.moviesLocation);
        if (NULL == loc)
        {
            Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
            return messageHandled;
        }
        ModuleDialog::handleLookupFinished(event, data);
        if (IDOK == ChangeLocationDialog::showModal(loc, handle()))
        {
            free(prefs.moviesLocation);
            prefs.moviesLocation = loc;
            if (NULL != prefs.moviesLocation)
                MoviesFetchData();
        }
        return messageHandled;
    }

    }
    return ModuleDialog::handleLookupFinished(event, data);
}

void MoviesMainDialog::setDisplayMode(DisplayMode dm)
{
    switch (displayMode_ = dm)
    {
    case showTheatres:
    case showMovies:
        renderer_.hide();
        list_.show();
        break;
    case showMovieDetails:
    case showTheatreDetails:
        list_.hide();
        renderer_.show();
        break;
    }
}

void MoviesMainDialog::prepareTheatresList()
{
}