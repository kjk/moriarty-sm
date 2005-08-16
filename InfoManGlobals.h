#ifndef INFOMAN_GLOBALS_H__
#define INFOMAN_GLOBALS_H__

#include <Debug.hpp>
#include <BaseTypes.hpp>

enum InfoManExtEvents {
	extEventLookupStarted,
	extEventLookupProgress,
	extEventLookupFinished
};
	
class Preferences;
class LookupManager;
class MainWindow;
class HyperlinkHandler;

Preferences* GetPreferences();
LookupManager* GetLookupManager();
HyperlinkHandler* GetHyperlinkHandler();
char_t* GetStorePath(const char_t* name);

#define INFOMAN_VERSION "1.6"

#ifdef _PALM_OS
#define PLATFORM_NAME "PalmOS"
#endif

#ifdef _WIN32_WCE
#ifdef WIN32_PLATFORM_PSPC
#define PLATFORM_NAME "PocketPC"
#endif
#ifdef WIN32_PLATFORM_WFSP
#define PLATFORM_NAME "Smartphone"
#endif
#endif

#define clientInfo PLATFORM_NAME " " INFOMAN_VERSION

#define SERVER_LOCALHOST "192.168.1.2:4000"
#define SERVER_ADDRESS SERVER_LOCALHOST


#define dataStreamPostfix        "data"
#define prefsStreamPostfix       "prefs"

#define globalPrefsStream prefsStreamPostfix

#endif // INFOMAN_GLOBALS_H__