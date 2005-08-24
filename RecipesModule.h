#ifndef INFOMAN_RECIPES_MODULE_H__
#define INFOMAN_RECIPES_MODULE_H__

#include "Modules.h"

//class ModuleDialog;
//ModuleDialog* RecipesStart();

MODULE_STARTER_DECLARE(Recipes);

struct UniversalDataFormat;
class DefinitionModel;
DefinitionModel* RecipeExtractFromUDF(const UniversalDataFormat& udf);

#endif