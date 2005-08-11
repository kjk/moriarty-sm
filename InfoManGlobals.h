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

#define INFOMAN_VERSION "1.6"

#ifdef _PALM_OS
#define clientInfo "PalmOS " INFOMAN_VERSION
#endif

#ifdef _WIN32_WCE
#ifdef WIN32_PLATFORM_PSPC
#define clientInfo "PocketPC " INFOMAN_VERSION
#endif
#ifdef WIN32_PLATFORM_WFSP
#define clientInfo "Smartphone " INFOMAN_VERSION
#endif
#endif

#define SERVER_LOCALHOST "127.0.0.1:4000"
#define SERVER_ADDRESS SERVER_LOCALHOST


#define dataStreamPostfix        "data"
#define prefsStreamPostfix       "prefs"

#define globalPrefsStream prefsStreamPostfix

#endif // INFOMAN_GLOBALS_H__