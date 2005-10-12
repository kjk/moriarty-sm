#include "MoviesMainDialog.h"
#include "LookupManager.h"
#include "InfoMan.h"
#include "InfoManPreferences.h"
#include "ChangeLocationDialog.h"
#include "MoviesModule.h"
#include "HyperlinkHandler.h"

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
    list_.setStyleEx(LVS_EX_GRADIENT | LVS_EX_FULLROWSELECT | LVS_EX_ONECLICKACTIVATE | LVS_EX_NOHSCROLL);
    list_.setTextBkColor(CLR_NONE);
    LVCOLUMN col = {0};
    col.mask = LVCF_TEXT;
    col.pszText = _T("");
    list_.insertColumn(0, col);
    
    Rect r;
    innerBounds(r);
    renderer_.create(WS_TABSTOP, r, handle());
    renderer_.definition.setHyperlinkHandler(GetHyperlinkHandler());

    ModuleDialog::handleInitDialog(fw, ip);
    
    setDisplayMode(displayMode_);
    
    UniversalDataFormat* udf = UDF_ReadFromStream(moviesDataStream);
    if (NULL != udf)
    {
        udf_ = udf;
        prepareTheatresList();
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
    list_.setColumnWidth(0, w);
    renderer_.anchor(anchorRight, 0, anchorBottom, 0, repaintWidget);
    return ModuleDialog::handleResize(st, w, h);
}

long MoviesMainDialog::handleCommand(ushort nc, ushort id, HWND sender)
{
    switch (id) {
    case IDM_MOVIES_MOVIES:
    case ID_VIEW_MOVIES:
        if (NULL == udf_)
            return messageHandled;
        if (movies_.empty())
        {
            if (errNone != MoviesFromTheatres(movies_, *udf_))
            {
                Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
                return messageHandled;
            }
        }
        prepareMoviesList();
        setDisplayMode(showMovies);
        return messageHandled;
    case IDM_MOVIES_THEATERS:
    case ID_VIEW_THEATERS:
        if (NULL == udf_)
            return messageHandled;
        prepareTheatresList();
        setDisplayMode(showTheatres);
        return messageHandled;
    case ID_VIEW_UPDATE:
        if (errNone != MoviesFetchData())
            Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
        return messageHandled;
    case ID_VIEW_CHANGE_LOCATION:
    {
        Preferences& prefs = *GetPreferences();
        char_t* loc = StringCopy(prefs.moviesLocation);
        if (IDOK == ChangeLocationDialog::showModal(loc, handle()))
        {
            free(prefs.moviesLocation);
            prefs.moviesLocation = loc;
            if (errNone != MoviesFetchData())
                Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
        }
        return messageHandled;
    }
    }
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
        renderer_.hide();
        list_.show();
        menuBar().replaceButton(IDM_MOVIES_THEATERS, IDM_MOVIES_MOVIES, IDS_MOVIES);
        break;
        
    case showMovies:
        renderer_.hide();
        list_.show();
        menuBar().replaceButton(IDM_MOVIES_MOVIES, IDM_MOVIES_THEATERS, IDS_THEATERS);
        break;
        
    case showMovieDetails:
        list_.hide();
        renderer_.show();
        menuBar().replaceButton(IDM_MOVIES_THEATERS, IDM_MOVIES_MOVIES, IDS_MOVIES);
        break;
        
    case showTheatreDetails:
        list_.hide();
        renderer_.show();
        menuBar().replaceButton(IDM_MOVIES_MOVIES, IDM_MOVIES_THEATERS, IDS_THEATERS);
        break;
    }
    HMENU menu = menuBar().subMenu(IDM_VIEW);
    UINT state = (udf_ != NULL ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(menu, ID_VIEW_MOVIES, state | MF_BYCOMMAND);
    EnableMenuItem(menu, ID_VIEW_THEATERS, state | MF_BYCOMMAND);
    if (showTheatres == dm)
        CheckMenuRadioItem(menu, ID_VIEW_MOVIES, ID_VIEW_THEATERS, ID_VIEW_THEATERS, MF_BYCOMMAND);
    else if (showMovies == dm)
        CheckMenuRadioItem(menu, ID_VIEW_MOVIES, ID_VIEW_THEATERS, ID_VIEW_MOVIES, MF_BYCOMMAND);
    else 
    {
        CheckMenuItem(menu, ID_VIEW_MOVIES, MF_UNCHECKED | MF_BYCOMMAND);
        CheckMenuItem(menu, ID_VIEW_THEATERS, MF_UNCHECKED | MF_BYCOMMAND);
    }
}

void MoviesMainDialog::prepareTheatresList()
{
    list_.clear();
    if (NULL == udf_)
        return;

    ulong_t size = udf_->getItemsCount() / 2;
    LVITEM item;
    ZeroMemory(&item, sizeof(item));
    for (ulong_t i = 0; i < size; ++i)
    {
        item.mask = LVIF_TEXT;
        item.iItem = i;
        const char_t* text = udf_->getItemText(i * 2, theatreNameIndex);
        item.pszText = const_cast<char_t*>(text);
        list_.insertItem(item);
    }
}

void MoviesMainDialog::prepareMoviesList()
{
    list_.clear();
    if (movies_.empty())
        return;

    ulong_t size = movies_.size();
    LVITEM item;
    ZeroMemory(&item, sizeof(item));
    for (ulong_t i = 0; i < size; ++i)
    {
        item.mask = LVIF_TEXT;
        item.iItem = i;
        const char_t* text = movies_[i]->title;
        item.pszText = const_cast<char_t*>(text);
        list_.insertItem(item);
    }
}

long MoviesMainDialog::handleListItemActivate(int controlId, const NMLISTVIEW &h)
{
    ulong_t item = h.iItem;
    if (showMovies == displayMode_)
        showMovie(item);
    else if (showTheatres == displayMode_)
        showTheatre(item);
    return messageHandled;
}

void MoviesMainDialog::showMovie(ulong_t index)
{
    DefinitionModel* model = MoviesPrepareMovie(movies_, index);
    if (NULL == model)
    {
        Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
        return;
    }
    renderer_.setModel(model, Definition::ownModel);
    setDisplayMode(showMovieDetails);
}

void MoviesMainDialog::showTheatre(ulong_t index)
{
    if (NULL == udf_)
        return;
        
    DefinitionModel* model = MoviesPrepareTheatre(*udf_, index);
    if (NULL == model)
    {
        Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
        return;
    }
    renderer_.setModel(model, Definition::ownModel);
    setDisplayMode(showTheatreDetails);
}
