#include "WeatherMainDialog.h"
#include "InfoMan.h"
#include "HyperlinkHandler.h"
#include "Modules.h"

using namespace DRA;

WeatherMainDialog::WeatherMainDialog():
    ModuleDialog(IDR_WEATHER_MENU)
{
}

MODULE_DIALOG_CREATE_IMPLEMENT(WeatherMainDialog, IDD_WEATHER_MAIN)

bool WeatherMainDialog::handleInitDialog(HWND wnd, long lp)
{
	Rect r;
	bounds(r);
	renderer_.definition.setHyperlinkHandler(GetHyperlinkHandler());
	renderer_.definition.setInteractionBehavior(0);
	renderer_.definition.setNavOrderOptions(Definition::navOrderFirst);
	// renderer_.create(WS_VISIBLE|WS_TABSTOP, SCALEX(1), SCALEY(1), r.width() - SCALEX(2), r.height() - SCALEY(2), handle());
	
    return ModuleDialog::handleInitDialog(wnd, lp);
}

bool WeatherMainDialog::handleLookupFinished(Event& event, const LookupFinishedEventData* data)
{
    
   return ModuleDialog::handleLookupFinished(event, data); 
}

long WeatherMainDialog::handleCommand(ushort notify_code, ushort id, HWND sender)
{
    if (IDOK == id)
    {
        ModuleRunMain();
        return messageHandled; 
    }  
	return ModuleDialog::handleCommand(notify_code, id, sender);
}

long WeatherMainDialog::handleResize(UINT sizeType, ushort width, ushort height)
{
	// renderer_.anchor(anchorRight, SCALEX(2), anchorBottom, SCALEY(2), repaintWidget);
	// tabs_.anchor(anchorRight, 0, anchorBottom, 0, repaintWidget);
	return ModuleDialog::handleResize(sizeType, width, height);
}
