#include "JokesMainDialog.h"
#include "LookupManager.h"
#include "InfoMan.h"
#include "InfoManPreferences.h"
#include "JokesModule.h"

#include <SysUtils.hpp>
#include <UniversalDataHandler.hpp>
#include <Utility.hpp>
#include <Text.hpp>
#include <UTF8_Processor.hpp>

enum {
    jokesCategoriesStart = IDC_CHECK1,
    jokesCategoriesCount = JokesPrefs::categoryCount,
    jokesExplicitnessStart = jokesCategoriesStart + jokesCategoriesCount,
    jokesExplicitnessCount = JokesPrefs::explicitnessCategoryCount,
    jokesTypesStart = jokesExplicitnessStart + jokesExplicitnessCount,
    jokesTypesCount = JokesPrefs::typeCount
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
    
    void loadSearchPrefs();
    void storeSearchPrefs();
    void prepareQuery();

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
        
        sort_.addString(IDS_SORT_RATING);
        sort_.addString(IDS_SORT_RANK);


        loadSearchPrefs();
        term_.focus();
        return false;
    }
    
    long handleCommand(ushort nc, ushort id, HWND sender)
    {
        switch (id) 
        {
            case IDOK:
                storeSearchPrefs();
                prepareQuery();
                // Intentional fall-through
            case IDCANCEL:
                endModal(id);
                return messageHandled;
        }
        return MenuDialog::handleCommand(nc, id, sender);
    }
    
    long handleResize(UINT, ushort, ushort)
    {
#ifdef WIN32_PLATFORM_PSPC
        if (-1 != clientHeight_)
        {

            SCROLLINFO si = {sizeof(si)};
            si.fMask = SIF_RANGE | SIF_PAGE;
            si.nPage = 120;
            si.nMin = 0;
            si.nMax = clientHeight_;
            SetScrollInfo(handle(), SB_VERT, &si, TRUE);
        }
#else
        sendMessage(DM_RESETSCROLL, FALSE, TRUE);
#endif
        return messageHandled;
    }
    
#ifdef WIN32_PLATFORM_PSPC
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
                if (newPos > si.nMax - int(si.nPage)) newPos = si.nMax - si.nPage;
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
#endif 

public:
    
    static long showModal(HWND parent, char*& query) 
    {
        JokesSearchDialog* dlg = new_nt JokesSearchDialog();
        if (NULL == dlg)
        {
            Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
            return IDCANCEL;
        }
        long res = dlg->MenuDialog::showModal(GetInstance(), IDD_JOKES_SEARCH, parent);
        if (IDOK == res)
        {
            free(query);
            query = dlg->query_;
            dlg->query_ = NULL;
        }
        delete dlg;
        return res;
    }
    
};


void JokesSearchDialog::storeSearchPrefs()
{
    JokesPrefs& prefs = GetPreferences()->jokesPrefs;
    bool check;
    for (ulong_t i = 0; i < jokesCategoriesCount; ++i)
    {
        check = (BST_CHECKED == SendMessage(child(jokesCategoriesStart + i), BM_GETCHECK, 0, 0));
        prefs.categories[i] = check;
    }
    for (ulong_t i = 0; i < jokesExplicitnessCount; ++i)
    {
        check = (BST_CHECKED == SendMessage(child(jokesExplicitnessStart + i), BM_GETCHECK, 0, 0));
        prefs.explicitnessCategories[i] = check;
    }
    for (ulong_t i = 0; i < jokesTypesCount; ++i)
    {
        check = (BST_CHECKED == SendMessage(child(jokesTypesStart + i), BM_GETCHECK, 0, 0));
        prefs.types[i] = check;
    }
    
    prefs.minimumRating = rating_.position() - 1;
    prefs.sortOrder = sort_.selection();
}

void JokesSearchDialog::loadSearchPrefs()
{
    JokesPrefs& prefs = GetPreferences()->jokesPrefs;
    rating_.setPosition(prefs.minimumRating + 1, repaintWidget);
    sort_.setSelection(prefs.sortOrder);
    for (ulong_t i = 0; i < jokesCategoriesCount; ++i)
        SendMessage(child(jokesCategoriesStart + i), BM_SETCHECK, prefs.categories[i] ? BST_CHECKED : BST_UNCHECKED, 0);

    for (ulong_t i = 0; i < jokesExplicitnessCount; ++i)
        SendMessage(child(jokesExplicitnessStart + i), BM_SETCHECK, prefs.explicitnessCategories[i] ? BST_CHECKED : BST_UNCHECKED, 0);


    for (ulong_t i = 0; i < jokesTypesCount; ++i)
        SendMessage(child(jokesTypesStart + i), BM_SETCHECK, prefs.types[i] ? BST_CHECKED : BST_UNCHECKED, 0);
}

void JokesSearchDialog::prepareQuery()
{
    free(query_);
    query_ = NULL;
    char_t* q = term_.text();
    if (NULL == q)
        return;

    ulong_t l = Len(q);
    for (ulong_t i = 0; i < l; ++i)
        if (_T(';') == q[i])
            q[i] = _T(' ');

    query_ = UTF8_FromNative(q);
    free(q);
    if (NULL == query_)
        Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
}

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
        {
            char* query = NULL;
            long res = JokesSearchDialog::showModal(handle(), query);
            sipPreference(SIP_FORCEDOWN);
            if (IDOK == res)
            {
                status_t err = JokesFetchQuery(query);
                free(query);
                if (errNone != err)
                    Alert(IDS_ALERT_NOT_ENOUGH_MEMORY);
            }
            return messageHandled;
        }
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