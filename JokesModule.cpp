#include "JokesModule.h"
#include "LookupManager.h"
#include "HyperlinkHandler.h"
#include "InfoManPreferences.h"

#include "MoriartyStyles.hpp"

#include <UniversalDataHandler.hpp>
#include <Definition.hpp>
#include <DefinitionElement.hpp>
#include <Text.hpp>

#ifdef _WIN32
#include "JokesMainDialog.h"
#endif

MODULE_STARTER_DEFINE(Jokes)

JokesPrefs::JokesPrefs():
    udf(NULL),
    minimumRating(0),
    sortOrder(0)
{
    for (ulong_t i = 0; i < ARRAY_SIZE(categories); ++i)
        categories[i] = true;
    for (ulong_t i = 0; i < ARRAY_SIZE(types); ++i)
        types[i] = true;
    for (ulong_t i = 0; i < ARRAY_SIZE(explicitnessCategories); ++i)
        explicitnessCategories[i] = true;
}

JokesPrefs::~JokesPrefs()
{
    delete udf;
}

void JokesPrefs::serializeVersion(Serializer &ser, ulong_t version)
{
    for (ulong_t i = 0; i < ARRAY_SIZE(types); ++i)
        ser(types[i]);

    // PalmOS InfoMan had hardcoded category count of 23, but since then it changed on the website to 24
    // so we increase scheme version number to support variable number of categories.
    ulong_t size = ARRAY_SIZE(categories);
    if (version > 1)
        ser(size);
    else 
        size = 23;
    assert(size <= ARRAY_SIZE(categories));

    for (ulong_t i = 0; i < size; ++i)
        ser(categories[i]);
        
    for (ulong_t i = 0; i < ARRAY_SIZE(explicitnessCategories); ++i)
        ser(explicitnessCategories[i]);

    ser(minimumRating);
    ser(sortOrder);
}

void JokesPrefs::serialize(Serializer &ser)
{
    serializeVersion(ser, schemaVersion());
}

bool JokesPrefs::serializeInFromVersion(Serializer &ser, ulong_t version)
{
    if (1 != version)
        return false;
    serializeVersion(ser, version);
    return true;
}

ulong_t JokesPrefs::schemaVersion() const 
{
    return 2;
}

enum {
    jokesListItemRankIndex,
    jokesListItemTitleIndex,
    jokesListItemRatingIndex,
    jokesListItemExplicitnessIndex,
    jokesListItemUrlIndex,
    jokesListItemElementsCount
};

enum {
    jokeTitleIndex,
    jokeTextIndex,
    jokeElementsCount
};

DefinitionModel* JokeExtractFromUDF(const UniversalDataFormat& udf)
{
    DefinitionModel* model = new_nt DefinitionModel();
    if (NULL == model)
        return NULL;
        
    if (errNone != model->appendText(udf.getItemText(0, jokeTitleIndex)))
        goto Error;
    model->last()->setStyle(StyleGetStaticStyle(styleNamePageTitle));
    model->last()->setJustification(DefinitionElement::justifyCenter);
  
    if (errNone != model->appendLineBreak())
        goto Error;
    if (errNone != model->appendLineBreak())
        goto Error;

    if (errNone != DefinitionParseSimple(*model, udf.getItemData(0, jokeTextIndex)))
        goto Error;

    return model;
Error:
    delete model;
    return NULL;
}

status_t JokesFetchRandom()
{
    LookupManager* lm = GetLookupManager();
    const char* url = urlSchemaJoke urlSeparatorSchemaStr "random";
    return lm->fetchUrl(url);
}

static const char* jokesCategories[]={
    "Blonde",
    "Entertainment",
    "Men/Women",
    "Insults",
    "Yo-Mama",
    "Lawyer",
    "News&Politics",
    "Redneck",
    "Barroom",
    "Gross",
    "Sports",
    "Foreign",
    "Whatever",
    "Medical",
    "Sexuality",
    "Animals",
    "Children",
    "Anti-Joke",
    "Bush",
    "College",
    "Farm",
    "Business",
    "Religious",
    "Tech"
};

static const char* jokesTypes[]={
    "Articles",
    "One-Liners",
    "QandA",
    "Sketches",
    "Stories",
    "Lists"
};

static const char* jokesExplicitnessCategories[]={
    "Clean",
    "Tame",
    "Racy"
};

static const char* jokesSortOrders[] = {
    "rating",
    "rank"
};

static StaticAssert<ARRAY_SIZE(jokesCategories) == JokesPrefs::categoryCount> jokes_category_names_valid;
static StaticAssert<ARRAY_SIZE(jokesTypes) == JokesPrefs::typeCount> jokes_type_names_valid;
static StaticAssert<ARRAY_SIZE(jokesExplicitnessCategories) == JokesPrefs::explicitnessCategoryCount> jokes_expl_names_valid;

status_t JokesFetchQuery(const char* query)
{
    LookupManager* lm = GetLookupManager();
    char* url = StringCopy(urlSchemaJokesList urlSeparatorSchemaStr);
    if (NULL == url)
        return memErrNotEnoughSpace;

    JokesPrefs& prefs = GetPreferences()->jokesPrefs;
    char buffer[16];
    StrPrintF(buffer, "%d", prefs.minimumRating);
    if (NULL == (url = StrAppend(url, -1, buffer, -1, "; ", -1)))
        return memErrNotEnoughSpace;

    if (NULL == (url = StrAppend(url, -1, jokesSortOrders[prefs.sortOrder], -1, "; ", -1)))
        return memErrNotEnoughSpace;
    
    for (ulong_t i = 0; i < prefs.explicitnessCategoryCount; ++i)
        if (prefs.explicitnessCategories[i] && NULL == (url = StrAppend(url, -1, jokesExplicitnessCategories[i], -1, " ", -1)))
            return memErrNotEnoughSpace;
    if (NULL == (url = StrAppend(url, -1, "; ", -1)))
        return memErrNotEnoughSpace;
        
    for (ulong_t i = 0; i < prefs.typeCount; ++i)
        if (prefs.types[i] && NULL == (url = StrAppend(url, -1, jokesTypes[i], -1, " ", -1)))
            return memErrNotEnoughSpace;
    if (NULL == (url = StrAppend(url, -1, "; ", -1)))
        return memErrNotEnoughSpace;
        
    for (ulong_t i = 0; i < prefs.categoryCount; ++i)
        if (prefs.categories[i] && NULL == (url = StrAppend(url, -1, jokesCategories[i], -1, " ", -1)))
            return memErrNotEnoughSpace;
    if (NULL == (url = StrAppend(url, -1, "; ", -1)))
        return memErrNotEnoughSpace;

    url = StrAppend(url, -1, query, -1);
    if (NULL == url)
        return memErrNotEnoughSpace;
        
    status_t err = lm->fetchUrl(url);
    free(url);
    return err;
}
