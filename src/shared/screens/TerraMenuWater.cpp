#include "TerraMenuScreens.h"
#include "TerraMenus.h"
#include "Terraduino.h"
#include "TerraStrings.h"
#include "TerraUIStrings.h"
#include "TerraUIData.h"
#include "TerraUIInlines.hh"

void terraBuildWaterScreen(const Terraduino &controller, TerraMenuModel &model) {
    model.clear("Water");
    for (uint8_t i=0;i<controller.objectCount();++i) { TerraObject *o=controller.objectAt(i); if(!o) continue;
        if(o->getObjectType()==Terra_ObjectType_WaterStorage) { const TerraWaterStorage *s=static_cast<const TerraWaterStorage*>(o); model.add(o->getName(), terraUIFloat(s->getLevel()) + "%"); }
        else if(o->getObjectType()==Terra_ObjectType_WaterRoute) { const TerraWaterRoute *r=static_cast<const TerraWaterRoute*>(o); model.add(o->getName(), terraRouteStateToString(r->getRouteState())); }
    }
    if(!model.rowCount()) model.add("Status","No water objects");
}
