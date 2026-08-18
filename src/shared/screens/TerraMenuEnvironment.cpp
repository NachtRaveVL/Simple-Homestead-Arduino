#include "TerraMenuScreens.h"
#include "TerraMenus.h"
#include "Terraduino.h"
#include "TerraStrings.h"
#include "TerraUIStrings.h"
#include "TerraUIData.h"
#include "TerraUIInlines.hh"

void terraBuildEnvironmentScreen(const Terraduino &controller, TerraMenuModel &model) {
    model.clear("Environment");
    const TerraObject *o=controller.findFirstByType(Terra_ObjectType_Environment); if(!o){model.add("Status","No environment"); return;}
    const TerraEnvironment *e=static_cast<const TerraEnvironment*>(o);
    if(e->hasField(TERRA_WEATHER_AIR_TEMPERATURE)) model.add("Temperature",terraUIFloat(e->getAirTemperature())+" C");
    if(e->hasField(TERRA_WEATHER_HUMIDITY)) model.add("Humidity",terraUIFloat(e->getRelativeHumidity())+"%");
    if(e->hasField(TERRA_WEATHER_PRESSURE)) model.add("Pressure",terraUIFloat(e->getBarometricPressure())+" hPa");
    if(e->hasField(TERRA_WEATHER_RAINFALL)) model.add("Rain",terraUIFloat(e->getRainfall())+" mm");
    if(e->hasField(TERRA_WEATHER_WIND_SPEED)) model.add("Wind",terraUIFloat(e->getWindSpeed())+" m/s");
}
