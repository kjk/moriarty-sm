#include "MoviesMainDialog.h"
#include "LookupManager.h"
#include "InfoMan.h"

#include <UniversalDataFormat.hpp>

MoviesMainDialog::MoviesMainDialog():
    ModuleDialog(IDR_MOVIES_MENU),
    udf_(NULL),
    displayMode_(showMovies)
{
}

MoviesMainDialog::~MoviesMainDialog()
{
    delete udf_;
    for (ulong_t i = 0; i < movies_.size(); ++i)
        delete movies_[i];
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
    //switch (data->result)
    //{
    //}
    return ModuleDialog::handleLookupFinished(event, data);
}
