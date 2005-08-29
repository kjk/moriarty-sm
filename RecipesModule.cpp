#include "RecipesModule.h"
#include "MoriartyStyles.hpp"
#include "LookupManager.h"
#include "InfoManPreferences.h"

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
#include <UniversalDataHandler.hpp>
#include <TextElement.hpp>
#include <LineBreakElement.hpp>
#include <Text.hpp>
#include <ByteFormatParser.hpp>

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
    const RecipesPrefs& prefs = GetPreferences()->recipesPrefs;
    
    DefinitionModel* model = new_nt DefinitionModel();
    if (NULL == model)
        return NULL;

   
    status_t err = errNone;
    TextElement* text = NULL;
    const char* str = NULL;
    if (prefs.activeSections[prefs.recipeName])
    {
        TXT(recipe.getItemText(0, recipeNameIndex));
        text->setStyle(StyleGetStaticStyle(styleNameBold));
        text->setJustification(DefinitionElement::justifyCenter);
    
        APP(new_nt LineBreakElement());
        APP(new_nt LineBreakElement());
    }    

    if (prefs.activeSections[prefs.recipeNote])
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

    if (prefs.activeSections[prefs.recipeIngredients])
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

    if (prefs.activeSections[prefs.recipePreperation])
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

    if (prefs.activeSections[prefs.recipeReviews])
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

    if (prefs.activeSections[prefs.recipeGlobalNote])
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


status_t RecipesDataRead(DefinitionModel*& listModel, DefinitionModel*& itemModel)
{
    UniversalDataFormat* udf = new_nt UniversalDataFormat();
    if (NULL == udf)
        return memErrNotEnoughSpace;
        
    status_t err = UDF_ReadFromStream(recipesListStream, *udf);
    if (errNone == err) 
    { 
        DefinitionModel* model = DefinitionModelFromUDF(*udf);
        if (NULL != model)
        {
            delete listModel;
            listModel = model;
        }
    }
   
    udf->reset();
    err = UDF_ReadFromStream(recipesItemStream, *udf);
    if (errNone == err)
    {
        DefinitionModel* model = RecipeExtractFromUDF(*udf);
        if (NULL != model)
        {
            delete itemModel;
            itemModel = model;
        }
    }     
    delete udf;
    return errNone;     
}

RecipesPrefs::RecipesPrefs()
{
    for (int i = 0; i < recipeSectionsCount_; ++i)
        activeSections[i] = true; 
    activeSections[recipeReviews] = false;
}

RecipesPrefs::~RecipesPrefs()
{
}
             
void RecipesPrefs::serialize(Serializer& ser)
{
    for (int i = 0; i < recipeSectionsCount_; ++i)
    {
        ser(activeSections[i]);
    }
}
