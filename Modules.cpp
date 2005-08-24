#include "Modules.h"
#include "InfoMan.h"

#include "WeatherModule.h"
#include "RecipesModule.h"

#ifdef _WIN32
#include "ModuleDialog.h"
#endif

#include <Text.hpp>

/*
struct Module {
    ModuleID id;
    const char* name;
    char_t* displayName    
    uint_t iconId;
#ifdef _PALM_OS
    uint_t largeIconId;
#endif
    ModuleStarter starter;
    ModuleDataReader dataReader; 
    bool free;
    bool dataReady;
    bool disabledRemotely;
    bool disabledByUser;
    bool tracksUpdateTime;
    tick_t lastUpdateTime;
    
    bool active() const {return !(disabledByUser || disabledRemotely);}    
   
    enum {neverUpdated == ulong_t(-1)};     
};
*/


#ifdef _PALM_OS
#define MOD(id, name, dispName, palmIcon, palmLargeIcon, winIcon, mainFormId, dataReader, free, tracksUpdate) \
{id, name, dispName, palmIcon, palmLargeIcon, mainFormId, dataReader, free, false, false, false, tracksUpdate, Module::neverUpdated}
#endif

// TODO: use commented-out version when data readers are ported to WinCE
#ifdef _WIN32

#define MOD_ICON_LARGE(name) IDB_ ## name ## _LARGE
#define MOD_ICON_SMALL(name) IDB_ ## name ## _SMALL

#define  MOD(id, name, dispName, palmIcon, palmLargeIcon, winIcon, mainFormId, starter, dataReader, free, tracksUpdate) \
{id, name, dispName, MOD_ICON_SMALL(winIcon), MOD_ICON_LARGE(winIcon), starter, NULL, free, false, false, false, tracksUpdate, Module::neverUpdated}
// {id, name, dispName, winIcon, starter, dataReader, free, false, false, false, tracksUpdate, neverUpdated}
#endif

static Module modules[] = {
    MOD(moduleIdWeather, weatherModuleName, _T("Weather"), weatherSmallBitmap, frmInvalidObjectId, NONE, weatherMainForm, WeatherStart, weatherDataRead, true, false),
    MOD(moduleId411, m411ModuleName, _T("Phone book"), m411SmallBitmap, frmInvalidObjectId, NONE, m411MainForm, NULL, m411DataRead, false, false),
    MOD(moduleIdMovies, moviesModuleName, _T("Movie times"), moviesSmallBitmap, frmInvalidObjectId, NONE, moviesMainForm, NULL, moviesDataRead, true, false),
    MOD(moduleIdAmazon, amazonModuleName, _T("Amazon"), amazonSmallBitmap, frmInvalidObjectId, NONE, amazonMainForm, NULL, NULL, false, false),
    MOD(moduleIdBoxOffice, boxOfficeModuleName, _T("Box office"), boxofficeSmallBitmap, frmInvalidObjectId, NONE, boxOfficeMainForm, NULL, boxOfficeDataRead, true, true),
    MOD(moduleIdCurrency, currencyModuleName, _T("Currency"), currencySmallBitmap, frmInvalidObjectId, NONE, currencyMainForm, NULL, currencyDataRead, true, false),
    MOD(moduleIdStocks, stocksModuleName, _T("Stocks"), stocksSmallBitmap, frmInvalidObjectId, NONE, stocksMainForm, NULL, stocksDataRead, true, false),
    MOD(moduleIdJokes, jokesModuleName, _T("Jokes"), jokesSmallBitmap, frmInvalidObjectId, NONE, jokesMainForm, NULL, jokesDataRead, false, true),
    MOD(moduleIdGasPrices, gasPricesModuleName, _T("Gas prices"), gasPricesSmallBitmap, frmInvalidObjectId, NONE, gasPricesMainForm, NULL, gasPricesDataRead, true, false),
    MOD(moduleIdHoroscopes, horoscopeModuleName, _T("Horoscopes"), horoscopesSmallBitmap, frmInvalidObjectId, NONE, horoscopesMainForm, NULL, horoscopeDataRead, true, true) ,
    MOD(moduleIdDreams, dreamsModuleName, _T("Dreams"), dreamsSmallBitmap, frmInvalidObjectId, NONE, dreamsMainForm, NULL, dreamsDataRead, false, true),
    MOD(moduleIdAbout, "about", _T("About"), aboutSmallBitmap, frmInvalidObjectId, NONE, frmInvalidObjectId, NULL, NULL, false, false),
#ifndef SHIPPING
    // this one's special: it never ships
    MOD(moduleIdTestWiki, "test", _T("Test parser"), aboutSmallBitmap, frmInvalidObjectId, NONE, testWikiMainForm, NULL, NULL, true, false),
#endif
#ifndef SHIPPING

    // WARNING!!!
    // When changing module to appear in "shipping" version remember to change this in MoriartyPreferences.cpp as well
    // (Preferences::serialize) and MoriartyApplication::createForm() in this file
    
    MOD(moduleIdNetflix, netflixModuleName, _T("Netflix"), netflixSmallBitmap, frmInvalidObjectId, NONE, netflixMainForm, NULL, netflixDataRead, false, false),
    MOD(moduleIdPedia, pediaModuleName, _T("Encyclopedia"), encyclopediaSmallBitmap, frmInvalidObjectId, NONE, pediaMainForm, NULL, NULL, true, false),
    MOD(moduleIdDict, dictModuleName, _T("Dictionary"), dictionarySmallBitmap, frmInvalidObjectId, NONE, dictMainForm, NULL, NULL, true, false),
    MOD(moduleIdLyrics, lyricsModuleName, _T("Lyrics"), lyricsSmallBitmap, frmInvalidObjectId, NONE, lyrics2MainForm, NULL, NULL, false, false),
    MOD(moduleIdRecipes, recipesModuleName, _T("Recipes"), epicuriousSmallBitmap, frmInvalidObjectId, RECIPES, epicuriousMainForm, RecipesStart, epicuriousDataRead, false, false),
    MOD(moduleIdListsOfBests, listsOfBestsModuleName, _T("Lists of bests"), listofbestsSmallBitmap, frmInvalidObjectId, NONE, listsOfBestsMainForm, NULL, NULL, false, false),
    // MOD(moduleIdTvListings, _T("TV Listings"), tvListingsSmallBitmap, frmInvalidObjectId, NONE, tvListingsMainForm, NULL, NULL, true, false),
    MOD(moduleIdQuotes, quotesModuleName, _T("Quotes"), quotationsSmallBitmap, frmInvalidObjectId, NONE, quotesMainForm, NULL, quotesDataRead, true, true),
    MOD(moduleIdEBooks, ebookModuleName, _T("eBooks"),  ebooksSmallBitmap, frmInvalidObjectId, NONE, ebookMainForm, NULL, NULL, false, false),
    MOD(moduleIdFlights, flightsModuleName, _T("Flights"),  flightsSmallBitmap, frmInvalidObjectId, NONE, flightsMainForm, NULL, flightsDataRead, true, false),
    MOD(moduleIdEBay, eBayModuleName, _T("eBay"), eBaySmallBitmap, frmInvalidObjectId, NONE, eBayMainForm, NULL, NULL, false, false),
    MOD(moduleIdFlickr, flickrModuleName, _T("Flickr"), flickrSmallBitmap, frmInvalidObjectId, NONE, flickrMainForm, NULL, NULL, false, true),
#endif
};

ulong_t ModuleCount() 
{
    return ARRAY_SIZE(modules);
};

ulong_t ModuleActiveCount()
{
    ulong_t res = 0;
    for (ulong_t i = 0; i < ARRAY_SIZE(modules); ++i)
        if (modules[i].active())
            ++res;
    return res;  
}

Module* ModuleGet(ulong_t index)
{
    assert(index < ARRAY_SIZE(modules));
    return modules + index; 
}

Module* ModuleGetByName(const char* name)
{
    for (ulong_t i = 0; i < ARRAY_SIZE(modules); ++i)
        if (StrEquals(name, modules[i].name)) 
            return modules + i;
    assert(false);
    return NULL;
}

Module* ModuleGetById(ModuleID id)
{
    for (ulong_t i = 0; i < ARRAY_SIZE(modules); ++i)
        if (id == modules[i].id)
            return modules + i;
    assert(false);
    return NULL; 
}

Module* ModuleGetActive(ulong_t index)
{
    ulong_t count = 0;
    for (ulong_t i = 0; i < ARRAY_SIZE(modules); ++i)
        if (modules[i].active())
        {
            if (count == index)
                return modules + i;
            else
                ++count;
        }
    assert(false);
    return NULL; 
}

static Module* runningModule = NULL;

Module* ModuleGetRunning()
{
    return runningModule;
}

ModuleID ModuleGetRunningId()
{
    if (NULL == runningModule)
        return moduleIdNone;
    
    return runningModule->id;  
}

status_t ModuleRun(ModuleID id)
{
    status_t err = errNone; 
    Module* mod = NULL;
    
    if (moduleIdNone != id)
    {  
        mod = ModuleGetById(id);
        if (NULL == mod || !mod->active())
            return sysErrParamErr;
    
        if (!mod->dataReady && NULL != mod->dataReader && errNone != (err = mod->dataReader()))
            return err;

    };
        
#ifdef _WIN32
    ModuleDialogDestroyCurrent();
    if (moduleIdNone != id)
    {
        if (NULL == mod->starter)
            return sysErrParamErr;
             
        ModuleDialog* d = mod->starter();
        if (NULL != d)
            d->show();
        else
        {
            DWORD err = GetLastError();
            assert(false);
            return sysErrParamErr;
            // TODO: show some alert?!
        }
        ModuleDialogSetCurrent(d); 
    }   
#endif
    
#ifdef _PALM_OS
    if (moduleIdNone != id)
        if (frmInvalidObjectId != mod->mainFormId)
            FrmGotoForm(mod->mainFormId);
        else
        {
            assert(moduleIdAbout == id);
            // TODO: show about form            
        }
    }
    else
        FrmGotoForm(mainForm);
#endif

    runningModule = mod;
    return errNone; 
}

void ModuleTouchRunning()
{
    if (NULL == runningModule)
        return;

    if (!runningModule->tracksUpdateTime)
        return;

    runningModule->lastUpdateTime = ticks();  
}
