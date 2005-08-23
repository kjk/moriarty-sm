#include "RecipesModule.h"
#include "RecipesMainDialog.h"

ModuleDialog* RecipesStart()
{
    return RecipesMainDialog::create();
}