#include "JokesModule.h"
#include "LookupManager.h"
#include "HyperlinkHandler.h"


#include "MoriartyStyles.hpp"

#include <UniversalDataHandler.hpp>
#include <Definition.hpp>
#include <DefinitionElement.hpp>

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