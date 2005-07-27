#ifndef INFOMAN_GLOBALS_H__
#define INFOMAN_GLOBALS_H__

enum InfoManExtEvents {
	extEventLookupStarted,
	extEventLookupProgress,
	extEventLookupFinished
};
	
class Preferences;
class LookupManager;

Preferences* GetPreferences();
LookupManager* GetLookupManager();

#endif // INFOMAN_GLOBALS_H__