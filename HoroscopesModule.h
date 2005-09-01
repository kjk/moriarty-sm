#ifndef HOROSCOPES_MODULE_H__
#define HOROSCOPES_MODULE_H__

#include "Modules.h"
#include <Serializer.hpp>

MODULE_STARTER_DECLARE(Horoscopes);

struct HoroscopesPrefs: public Serializable {
    
   HoroscopesPrefs();
   ~HoroscopesPrefs();
   
   char* finishedQuery;
   char* pendingQuery;
   ulong_t finishedSign;
   ulong_t pendingSign;
   enum {signNotSet = ulong_t(-1)};
   
   void serialize(Serializer& ser);
   
}; 

struct UniversalDataFormat;
class DefinitionModel;
DefinitionModel* HoroscopeExtractFromUDF(const UniversalDataFormat& udf, char_t*& date);

enum {horoscopesSignCount = 12};
status_t HoroscopeFetch(uint_t index);
status_t HoroscopeFetch(const char* query);

#endif // HOROSCOPES_MODULE_H__