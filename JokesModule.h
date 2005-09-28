#ifndef JOKES_MODULE_H__
#define JOKES_MODULE_H__

#include "Modules.h"
#include <Serializer.hpp>

MODULE_STARTER_DECLARE(Jokes);

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
};

#endif // JOKES_MODULE_H__