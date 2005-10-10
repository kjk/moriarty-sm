#include "MoviesModule.h"
#include "HyperlinkHandler.h"

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

