#include "JokesModule.h"
#include "LookupManager.h"
#include "HyperlinkHandler.h"

#include <UniversalDataFormat.hpp>

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

void JokesPrefs::serialize(Serializer &ser)
{
    for (ulong_t i = 0; i < ARRAY_SIZE(types); ++i)
        ser(types[i]);

    for (ulong_t i = 0; i < ARRAY_SIZE(categories); ++i)
        ser(categories[i]);

    for (ulong_t i = 0; i < ARRAY_SIZE(explicitnessCategories); ++i)
        ser(explicitnessCategories[i]);

    ser(minimumRating);
    ser(sortOrder);
}

DefinitionModel* JokeExtractFromUDF(const UniversalDataFormat& udf)
{
    // TODO: implement JokeExtractFromUDF()
    return NULL;
}

status_t JokesFetchRandom()
{
    LookupManager* lm = GetLookupManager();
    const char* url = urlSchemaJoke urlSeparatorSchemaStr "random";
    return lm->fetchUrl(url);
}