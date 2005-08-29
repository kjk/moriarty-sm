#ifndef INFOMAN_RECIPES_MODULE_H__
#define INFOMAN_RECIPES_MODULE_H__

#include "Modules.h"
#include <Serializer.hpp>

//class ModuleDialog;
//ModuleDialog* RecipesStart();

MODULE_STARTER_DECLARE(Recipes);

struct UniversalDataFormat;
class DefinitionModel;
DefinitionModel* RecipeExtractFromUDF(const UniversalDataFormat& udf);

status_t RecipesDataRead(DefinitionModel*& listModel, DefinitionModel*& itemModel);

struct RecipesPrefs: public Serializable {

    enum RecipePartIndex
    {
        recipeName,
        recipeNote,
        recipeIngredients,
        recipePreperation,
        recipeReviews,
        recipeGlobalNote,
        recipeSectionsCount_
    };
        
    RecipesPrefs();
    ~RecipesPrefs();
             
    bool activeSections[recipeSectionsCount_];

    void serialize(Serializer& ser);

};

#endif