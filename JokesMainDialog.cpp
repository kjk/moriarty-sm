#include "JokesMainDialog.h"
#include "LookupManager.h"
#include "InfoMan.h"
#include "InfoManPreferences.h"

#include <SysUtils.hpp>
#include <UniversalDataHandler.hpp>
#include <Utility.hpp>

enum {
    jokesCategoriesStart = IDC_CHECK1,
    jokesCategoriesCount = 24,
    jokesExplicitnessStart = jokesCategoriesStart + jokesCategoriesCount,
    jokesExplicitnessCount = 3,
    jokesTypesStart = jokesExplicitnessStart + jokesExplicitnessCount,
    jokesTypesCount = 6
};

class JokesSearchDialog: public MenuDialog {

    EditBox term_;
    Slider  rating_;
    ComboBox sort_;
    char* query_;
    long clientHeight_;
    
    JokesSearchDialog():
        MenuDialog(IDR_DONE_CANCEL_MENU),
        query_(NULL),
        clientHeight_(-1)
    {
        setAutoDelete(autoDeleteNot);
    }
    
    ~JokesSearchDialog()
    {
        free(query_);
    }

protected:
    
    bool handleInitDialog(HWND fw, long ip)
    {
        Rect r;
        innerBounds(r);
        clientHeight_ = r.height();
        
        term_.attachControl(handle(), IDC_SEARCH_TERM);
        sort_.attachControl(handle(), IDC_SORT_COMBO);
        rating_.attachControl(handle(), IDC_RATING_SLIDER);
        rating_.setRange(1, 5, repaintNot);
        rating_.setBuddy(rating_.buddyLeft, child(IDC_JOKES_RATING_1));
        rating_.setBuddy(rating_.buddyRight, child(IDC_JOKES_RATING_5));
        
        createSipPrefControl();
        
        MenuDialog::handleInitDialog(fw, ip);
        
        return false;
    }
    
    long handleCommand(ushort nc, ushort id, HWND sender)
    {
        switch (id) 
        {
            case IDCANCEL:
            case IDOK:
                endModal(id);
                return messageHandled;
        }
        return MenuDialog::handleCommand(nc, id, sender);
    }
    
    long handleResize(UINT, ushort, ushort)
    {
        if (-1 != clientHeight_)
        {
            
            SCROLLINFO si = {sizeof(si)};
            si.fMask = SIF_RANGE | SIF_PAGE;
            si.nPage = 120;
            si.nMin = 0;
            si.nMax = clientHeight_;
            SetScrollInfo(handle(), SB_VERT, &si, TRUE);
        }
        
        sendMessage(DM_RESETSCROLL, FALSE, TRUE);
        return messageHandled;
    }
    
    LRESULT callback(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg) 
        {
            case WM_VSCROLL:
            {
                int code = LOWORD(wParam);
                int nPos = (short int)HIWORD(wParam);
                HWND sb = (HWND)lParam;
                SCROLLINFO si = {sizeof(si), SIF_PAGE|SIF_POS|SIF_RANGE|SIF_TRACKPOS, 0, 0, 0, 0, 0};
                GetScrollInfo(sb, SB_VERT, &si);
                int newPos = si.nPos;
                switch (code) 
                {
                    case SB_BOTTOM:
                        newPos = si.nMax - si.nPage;
                        break;
                    case SB_TOP:
                        newPos = si.nMin;
                        break;
                    
                    case SB_THUMBTRACK:
                        newPos  = si.nTrackPos;
                        break;
                        
                    case SB_THUMBPOSITION:
                        newPos = nPos + si.nMin;
                        break;
                    
                    case SB_LINEDOWN:
                        newPos += 10;
                        break;
                    
                    case SB_LINEUP:
                        newPos -= 10;
                        break;
                    
                    case SB_PAGEDOWN:
                        newPos += si.nPage;
                        break;
                    
                    case SB_PAGEUP:
                        newPos -= si.nPage;
                        break;
                }
                if (newPos < 0) newPos = 0;
                if (newPos > si.nMax - si.nPage) newPos = si.nMax - si.nPage;
                si.fMask = SIF_POS;
                if (si.nPos != newPos)
                {
                    int dy = si.nPos - newPos;
                    si.nPos = newPos;
                    SetScrollInfo(sb, SB_VERT, &si, TRUE);
                    ScrollWindowEx(handle(), 0, dy, NULL, NULL, NULL, NULL, SW_INVALIDATE | SW_SCROLLCHILDREN);
                }
                return messageHandled;
            }
        }
        return MenuDialog::callback(uMsg, wParam, lParam);
    }
    
public:
    
    static long showModal(HWND parent) 
    {
        JokesSearchDialog dlg;
        return dlg.MenuDialog::showModal(GetInstance(), IDD_JOKES_SEARCH, parent);
    }
    
};

JokesMainDialog::JokesMainDialog():
    ModuleDialog(IDR_JOKES_MENU),
    displayMode_(showJoke)
{
    setMenuBarFlags(SHCMBF_HIDESIPBUTTON);
    renderer_.definition.setInteractionBehavior(Definition::behavUpDownScroll | Definition::behavMouseSelection);
}

JokesMainDialog::~JokesMainDialog()
{
}

MODULE_DIALOG_CREATE_IMPLEMENT(JokesMainDialog, IDD_JOKES_MAIN) 

bool JokesMainDialog::handleInitDialog(HWND fw, long ip)
{
    Rect r;
    innerBounds(r);
    renderer_.create(WS_TABSTOP, r, handle(), NULL);
    list_.attachControl(handle(), IDC_JOKE_LIST);

    ModuleDialog::handleInitDialog(fw, ip);
    setDisplayMode(displayMode_);
    
    bool empty = true;
    JokesPrefs& prefs = GetPreferences()->jokesPrefs;
    if (NULL == prefs.udf)
    {
        prefs.udf = UDF_ReadFromStream(jokesJokesListStream);
        if (NULL != prefs.udf)
        {
            setDisplayMode(showList);
            empty = false;
        }
    }
    
    UniversalDataFormat* udf = UDF_ReadFromStream(jokesJokeStream);
    if (NULL != udf)
    {
        DefinitionModel* model = JokeExtractFromUDF(*udf);
        if (NULL != model)
        {
            renderer_.setModel(model, Definition::ownModel);
            setDisplayMode(showJoke);
            empty = false;
        }
    }
    
    if (empty && errNone != JokesFetchRandom())
        Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
    
    return false;
}

void JokesMainDialog::setDisplayMode(DisplayMode dm)
{
    switch (displayMode_ = dm)
    {
        case showJoke:
            list_.hide();
            renderer_.show();
            renderer_.focus();
            break;
        case showList:
        {
            renderer_.hide();
            list_.show();
            list_.focus();
            long sel = list_.selection();
            if (-1 == sel)
                sel = 0;
            if (0 != list_.itemCount())
                list_.focusItem(sel);
            break;
        }
    }
    resyncViewMenu();
}

void JokesMainDialog::resyncViewMenu()
{
    HMENU menu = menuBar().subMenu(IDM_VIEW);
    CheckMenuRadioItem(menu, ID_VIEW_JOKE, ID_VIEW_JOKE_LIST, (showJoke == displayMode_ ? ID_VIEW_JOKE : ID_VIEW_JOKE_LIST), MF_BYCOMMAND);
    JokesPrefs& prefs = GetPreferences()->jokesPrefs;
    UINT state = MF_BYCOMMAND | (NULL == prefs.udf ? MF_GRAYED : MF_ENABLED);
    EnableMenuItem(menu, ID_VIEW_JOKE_LIST, state);
    state = MF_BYCOMMAND | (renderer_.definition.empty() ? MF_GRAYED : MF_ENABLED);
    EnableMenuItem(menu, ID_VIEW_JOKE, state);
}

long JokesMainDialog::handleResize(UINT st, ushort w, ushort h)
{
    renderer_.anchor(anchorRight, 0, anchorBottom, 0, repaintWidget);
    list_.anchor(anchorRight, 0, anchorBottom, 0, repaintWidget);
    
    return messageHandled;
}

long JokesMainDialog::handleCommand(ushort nc, ushort id, HWND sender)
{
    switch (id) 
    {
        case IDM_JOKES_RANDOM:
            if (errNone != JokesFetchRandom())
                Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
            return messageHandled;
            
        case ID_VIEW_JOKE:
            if (showJoke != displayMode_)
                setDisplayMode(showJoke);
            return messageHandled;

        case ID_VIEW_JOKE_LIST:
            if (showList != displayMode_)
                setDisplayMode(showList);
            return messageHandled;
        
        case ID_VIEW_FIND_JOKE:
            JokesSearchDialog::showModal(handle());
            return messageHandled;
    }
    return ModuleDialog::handleCommand(nc, id, sender);
}

bool JokesMainDialog::handleLookupFinished(Event& event, const LookupFinishedEventData* data)
{
    JokesPrefs& prefs = GetPreferences()->jokesPrefs;
    LookupManager& lm = *GetLookupManager();
    UniversalDataFormat* udf = NULL;
    switch (data->result)
    {
        case lookupResultJokesList:
            PassOwnership(lm.udf, prefs.udf);
            assert(NULL != prefs.udf);
            createListItems();
            setDisplayMode(showList);
            return true;
        
        case lookupResultJoke:
        {
            PassOwnership(lm.udf, udf);
            assert(NULL != udf);
            DefinitionModel* model = JokeExtractFromUDF(*udf);
            delete udf;
            if (NULL == model)
            {
                Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
                return true;
            }
            renderer_.setModel(model, Definition::ownModel);
            setDisplayMode(showJoke);
            return true;
        }
    }
    return ModuleDialog::handleLookupFinished(event, data);
}

void JokesMainDialog::createListColumns()
{
}

void JokesMainDialog::createListItems()
{
}