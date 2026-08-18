#include "TerraMenuScreens.h"
#include "TerraMenus.h"
#include "Terraduino.h"
#include "TerraStrings.h"
#include "TerraUIStrings.h"
#include "TerraUIData.h"
#include "TerraUIInlines.hh"

void terraBuildPowerRailsScreen(const Terraduino &controller, TerraMenuModel &model) {
    model.clear("Power Rails"); for(uint8_t i=0;i<controller.objectCount();++i){ TerraObject *o=controller.objectAt(i); if(!o||o->getObjectType()!=Terra_ObjectType_PowerRail) continue;
        const TerraPowerRail *r=static_cast<const TerraPowerRail*>(o); model.add(o->getName(), terraUIFloat(r->getMeasuredVoltage())+" V"); }
    if(!model.rowCount()) model.add("Status","No power rails");
}
