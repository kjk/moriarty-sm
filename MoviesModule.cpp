#include "MoviesModule.h"
#include "HyperlinkHandler.h"
#include "InfoManPreferences.h"
#include "LookupManager.h"

#include "MoriartyStyles.hpp"

#include <Definition.hpp>
#include <DefinitionStyle.hpp>
#include <BulletElement.hpp>
#include <UniversalDataFormat.hpp>
#include <UTF8_Processor.hpp>
#include <Text.hpp>

#ifdef _WIN32
#include "MoviesMainDialog.h"
#endif

MODULE_STARTER_DEFINE(Movies);

void HyperlinkHandler::handleMovie(const char* link,  ulong_t length, const Point*)
{
    //MoriartyApplication& app = MoriartyApplication::instance();
    //MoviesMainForm* form = static_cast<MoviesMainForm*>(app.getOpenForm(moviesMainForm));
    //if (NULL == form || NULL == form->theatres)
    //{
    //    assert(false);
    //    return;
    //}
    //const char* data = hyperlinkData(link, length);
    //Movies_t& movies = form->movies;
    //const UniversalDataFormat& theatres = *form->theatres;
    //if (movies.empty() && !theatres.empty())
    //    createMoviesFromTheatres(movies, theatres);
    //uint_t movie=std::find_if(movies.begin(), movies.end(), MovieTitleEquals(String(data, length))) - movies.begin();
    //sendEvent(MoriartyApplication::appSelectMovieEvent, MoriartyApplication::SelectItemEventData(movie));
}

//struct MovieTitleEquals {
//    const ArsLexis::String& title;
//    explicit MovieTitleEquals(const ArsLexis::String& t): title(t) {}
//    bool operator()(const Movie* m) const
//    {return m->title == title;}
//};

void HyperlinkHandler::handleTheatre(const char* link,  ulong_t length, const Point*)
{
    //MoriartyApplication& app = MoriartyApplication::instance();
    //MoviesMainForm* form = static_cast<MoviesMainForm*>(app.getOpenForm(moviesMainForm));
    //if (NULL == form || NULL == form->theatres)
    //{
    //    assert(false);
    //    return;
    //}
    //const char* data = hyperlinkData(link, length);
    //int theatre = -1;
    //const UniversalDataFormat& theatres = *form->theatres;
    //const uint_t theatresCount = theatres.getItemsCount()/2;
    //for (uint_t i = 0; i< theatresCount; ++i)
    //    if (StrEquals(theatres.getItemText(2*i, theatreNameIndex), data))
    //        theatre = i;
    //assert(-1 != theatre);
    //sendEvent(MoriartyApplication::appSelectTheatreEvent, MoriartyApplication::SelectItemEventData(theatre));
}


#define TXT(t) if (errNone != model->appendText(t)) goto Error
#define LBR() if (errNone != model->appendLineBreak()) goto Error
#define BUL() if (errNone != model->appendBullet()) goto Error

DefinitionModel* MoviesPrepareMovie(const Movies_t& movies, ulong_t index)
{
    status_t err = errNone;
    assert(index < movies.size());
    const Movie& mv=*(movies[index]);
    
    DefinitionModel* model = new_nt DefinitionModel();
    if (NULL == model)
        return NULL;

    TXT(mv.title);
    model->last()->setStyle(StyleGetStaticStyle(styleNamePageTitle));

    LBR();

    ulong_t size = mv.theatres.size();
    for (ulong_t i = 0; i < size; ++i)
    {
        const TheatreByMovie& th=*mv.theatres[i];
        DefinitionElement* bull = NULL;
        BUL();
        bull = model->last();
        bull->setStyle(StyleGetStaticStyle(styleNameHeader));
        TXT(th.name);
        model->last()->setParent(bull);
        model->last()->setStyle(StyleGetStaticStyle(styleNameHeader));
        char* name = UTF8_FromNative(th.name);
        if (NULL == name)
            goto Error;
            
        char* link = StrAppend<char>(NULL, -1, urlSchemaTheatre urlSeparatorSchemaStr, -1, name, -1);
        free(name);
        if (NULL == link)
            goto Error;
        
        err = model->last()->setHyperlink(link, hyperlinkUrl);
        free(link);
        if (errNone != err)
            goto Error;
        
        LBR();
        model->last()->setParent(bull);
        TXT(th.hours);
        model->last()->setParent(bull);
    }
    return model;
Error:
    delete model;
    return NULL;
}


DefinitionModel* MoviesPrepareTheatre(const UniversalDataFormat& theatres, ulong_t index)
{
    uint_t theatreRow = 2 * index;
    assert(theatreRow < theatres.getItemsCount());
    
    DefinitionModel* model = new_nt DefinitionModel();
    if (NULL == model)
        return NULL;
        
    const char_t* theatreName = theatres.getItemText(theatreRow, theatreNameIndex);
    TXT(theatreName);
    model->last()->setStyle(StyleGetStaticStyle(styleNamePageTitle));

    const char_t* theatreAddress = theatres.getItemText(theatreRow, theatreAddressIndex);
    if (0 != Len(theatreAddress))
    {
        LBR();
        TXT(theatreAddress);
    }
    LBR();
    const uint_t moviesRow = theatreRow + 1;
    const uint_t moviesCount = theatres.getItemElementsCount(moviesRow) / 2;
    for (uint_t i = 0; i<moviesCount; ++i)
    {   
        status_t err;
        DefinitionElement* bull;
        BUL();
        bull = model->last();
        bull->setStyle(StyleGetStaticStyle(styleNameHeader));
        
        
        const char_t* movieTitle = theatres.getItemText(moviesRow, i * 2);
        TXT(movieTitle);
        model->last()->setParent(bull);
        model->last()->setStyle(StyleGetStaticStyle(styleNameHeader));

        char* title = UTF8_FromNative(movieTitle);
        if (NULL == title)
            goto Error;
        char* url = StrAppend<char>(NULL, -1, urlSchemaMovie urlSeparatorSchemaStr, -1, title, -1);
        free(title);
        if (NULL == url)
            goto Error;
            
        err = model->last()->setHyperlink(url, hyperlinkUrl);
        free(url);
        if (errNone != err)
            goto Error;
            
        LBR();
        model->last()->setParent(bull);
        const char_t* movieHours = theatres.getItemText(moviesRow, i * 2 + 1);
        TXT(movieHours);
        model->last()->setParent(bull);
    }
    return model;
Error:
    delete model;
    return NULL;
}

status_t MoviesFetchData()
{
    Preferences& prefs = *GetPreferences();
    char* loc = UTF8_FromNative(prefs.moviesLocation);
    if (NULL == loc)
        return memErrNotEnoughSpace;

    char* url = StrAppend<char>(NULL, -1, urlSchemaMoviesGet urlSeparatorSchemaStr, -1, loc, -1);
    free(loc);
    if (NULL == url)
        return memErrNotEnoughSpace;
    
    LookupManager* lm = GetLookupManager();
    status_t err = lm->fetchUrl(url);
    free(url);
    return err;
}