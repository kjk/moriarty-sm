#ifndef JOKES_MODULE_H__
#define JOKES_MODULE_H__

#include "Modules.h"
#include <Serializer.hpp>

MODULE_STARTER_DECLARE(Jokes);

struct UniversalDataFormat;

struct JokesPrefs: public Serializable {

    enum {
        categoryCount = 24,
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
    
    ulong_t schemaVersion() const;
    bool serializeInFromVersion(Serializer& ser, ulong_t version);

private:
    
    void serializeVersion(Serializer& ser, ulong_t version);
    
};

class DefinitionModel;
DefinitionModel* JokeExtractFromUDF(const UniversalDataFormat& udf);

status_t JokesFetchRandom();
status_t JokesFetchQuery(const char* query);

#endif // JOKES_MODULE_H__