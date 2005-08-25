#include "RecipesModule.h"
#include "MoriartyStyles.hpp"

#include <ExtendedEvent.hpp>

#ifdef _WIN32
#include "RecipesMainDialog.h"
#endif

 MODULE_STARTER_DEFINE(Recipes)

//ModuleDialog* RecipesStart()
//{
//    RecipesMainDialog::showModal(ExtEventGetWindow());
//    return NULL ;
//}

#include <Definition.hpp>
#include <UniversalDataFormat.hpp>
#include <TextElement.hpp>
#include <LineBreakElement.hpp>
#include <Text.hpp>

enum {
    recipeNameIndex,
    recipeInfoIndex,
    recipeIngredientsIndex,
    recipePreperationIndex,
    recipeNoteIndex,
    recipeTupleItemsCount
};

enum {   
    reviewPersonIndex,
    reviewDateIndex,
    reviewNoteIndex,
    reviewTextIndex, 
    reviewTupleItemsCount
};

static TextElement* TextEl(const char_t* text)
{
    if (NULL == text)
        return NULL;
        
    TextElement* el = new_nt TextElement();
    if (NULL == el)
        return NULL;

    status_t err = errNone;
    ErrTry {
        el->setText(text);
    }
    ErrCatch(ex) {
        err = ex;
    } ErrEndCatch
    if (errNone != err)
    {
        delete el;
        return NULL;
    }
    return el;               
}

#define APP(elem) if (errNone != (err = model->append((elem)))) goto Error
#define TXT(t) APP(text = TextEl((t)))

DefinitionModel* RecipeExtractFromUDF(const UniversalDataFormat& recipe)
{
    // TODO: customize recipe based on preferences
    //const Preferences::EpicuriousPreferences& prefs = application().preferences().epicuriousPreferences; 
    
    DefinitionModel* model = new_nt DefinitionModel();
    if (NULL == model)
        return NULL;

   
    status_t err = errNone;
    TextElement* text = NULL;
    const char* str = NULL;
    //if (prefs.fDisplayRecipePart[prefs.recipeName])
    {
        TXT(recipe.getItemText(0, recipeNameIndex));
        text->setStyle(StyleGetStaticStyle(styleNameBold));
        text->setJustification(DefinitionElement::justifyCenter);
    
        APP(new_nt LineBreakElement());
        APP(new_nt LineBreakElement());
    }    

    //if (prefs.fDisplayRecipePart[prefs.recipeNote])
    {
        str = recipe.getItemData(0, recipeInfoIndex);
        if (0 != Len(str))
        {
            // avoid displaying huge empty space due to line breaks
            // if we don't have recipe info
            // TODO: maybe the test for empty string should be more
            // sophisticated
            if (errNone != (err = DefinitionParseSimple(*model, str, -1)))
                goto Error;

            APP(new_nt LineBreakElement());
            APP(new_nt LineBreakElement());
        }
    }    

    //if (prefs.fDisplayRecipePart[prefs.recipeIngredients])
    {
        TXT(_T("Ingredients"));
        text->setStyle(StyleGetStaticStyle(styleNameHeader));
        APP(new_nt LineBreakElement());

        str = recipe.getItemData(0, recipeIngredientsIndex);
        if (errNone != (err = DefinitionParseSimple(*model, str, -1)))
            goto Error;
        APP(new_nt LineBreakElement());
        APP(new_nt LineBreakElement());
    }    

    //if (prefs.fDisplayRecipePart[prefs.recipePreperation])
    {
        TXT(_T("Preparation"));
        text->setStyle(StyleGetStaticStyle(styleNameHeader));
        APP(new_nt LineBreakElement());

        str = recipe.getItemData(0, recipePreperationIndex);
        if (errNone != (err = DefinitionParseSimple(*model, str, -1)))
            goto Error;
        APP(new_nt LineBreakElement());
        APP(new_nt LineBreakElement());
    }    

    //if (prefs.fDisplayRecipePart[prefs.recipeReviews])
    {
        uint_t itemsCount = recipe.getItemsCount();
        if (itemsCount > 1)
        {
            TXT(_T("Reviews"));
            text->setStyle(StyleGetStaticStyle(styleNameHeader));
            APP(new_nt LineBreakElement());
        }
        
        for (uint_t i = 1; i < itemsCount; ++i) 
        {
            TXT(_T("Reviewed by: "));
            TXT(recipe.getItemText(i, reviewPersonIndex));
            APP(new_nt LineBreakElement());
            TXT(_T("Review: "));
            TXT(recipe.getItemText(i, reviewTextIndex));
            APP(new_nt LineBreakElement());
            TXT(_T("Review date: "));
            TXT(recipe.getItemText(i, reviewDateIndex));
            APP(new_nt LineBreakElement());
            long note = recipe.getItemNumericValue(i, reviewNoteIndex);
            if (note > 0)
            {
                TXT(_T("Note: "));
                TXT(recipe.getItemText(i, reviewNoteIndex));
                APP(new_nt LineBreakElement());
            }            
            APP(new_nt LineBreakElement());
        }
    }    

    //if (prefs.fDisplayRecipePart[prefs.recipeGlobalNote])
    {
        TXT(_T("Global note"));
        text->setStyle(StyleGetStaticStyle(styleNameHeader));
        APP(new_nt LineBreakElement());
        TXT(recipe.getItemText(0, recipeNoteIndex));
    }
    return model;     
Error:
    if (NULL != model)
        delete model;
    return NULL;     
}
