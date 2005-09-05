#ifndef INFOMAN_PREFERENCES_H__
#define INFOMAN_PREFERENCES_H__

#include "RecipesModule.h"
#include "HoroscopesModule.h"
#include "DreamsModule.h"

#include <Utility.hpp>

class Serializer;

class Preferences: private NonCopyable {
public:

    RecipesPrefs recipesPrefs;
    HoroscopesPrefs horoscopesPrefs; 
    DreamsPrefs dreamsPrefs; 

	Preferences();

	~Preferences();
	
	char* serverAddress;
    enum {cookieLength=32};
    enum {regCodeLength=32};
	char* cookie;
	char* regCode;
	char* lastClientVersion;
	
	status_t serialize(Serializer& ser);

};

void PrefsDispose();
void PrefsLoad();
void PrefsSave();

#endif