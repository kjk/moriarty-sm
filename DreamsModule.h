#ifndef DREAMS_MODULE_H__
#define DREAMS_MODULE_H__

#include "Modules.h"
#include <Serializer.hpp>

MODULE_STARTER_DECLARE(Dreams);

class DefinitionModel;
struct UniversalDataFormat;
DefinitionModel* DreamExtractFromUDF(const UniversalDataFormat& udf, char_t*& title);

class DreamsPrefs: public Serializable {
public:
    char_t* downloadedTerm;
    char_t* pendingTerm;
    
    DreamsPrefs();
    ~DreamsPrefs();
   
    void serialize(Serializer& ser);
};

status_t DreamsDataRead(DefinitionModel*& model, char_t*& title);
     
#endif // DREAMS_MODULE_H__