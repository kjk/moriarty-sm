#include "WeatherModule.h"
#include "WeatherMainDialog.h"

ModuleDialog* WeatherStart()
{
    return WeatherMainDialog::create();
}
