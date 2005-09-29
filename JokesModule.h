#ifndef JOKES_MODULE_H__
#define JOKES_MODULE_H__

#include "Modules.h"
#include <Serializer.hpp>

MODULE_STARTER_DECLARE(Jokes);

struct UniversalDataFormat;

struct JokesPrefs: public Serializable {

    enum {
        categoryCount = 23,
        typeCount = 6,
        explicitnessCategoryCount = 3
    };
    
    bool categories[categoryCount];
    bool types[typeCount];
    bool explicitnessCategories[explicitnessCategoryCount];
    
    int minimumRating;
    int sortOrder;
    
    JokesPrefs();
    ~JokesPrefs();
    
    void serialize(Serializer& ser);
    
    UniversalDataFormat* udf;
};

class DefinitionModel;
DefinitionModel* JokeExtractFromUDF(const UniversalDataFormat& udf);

status_t JokesFetchRandom();

#endif // JOKES_MODULE_H__