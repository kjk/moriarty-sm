#ifndef INFOMAN_GLOBALS_H__
#define INFOMAN_GLOBALS_H__

enum InfoManExtEvents {
	extEventText,
	extEventLookupFirst // This should be always last one
};
	
class Preferences;
class LookupManager;

Preferences* GetPreferences();
LookupManager* GetLookupManager();

#endif // INFOMAN_GLOBALS_H__