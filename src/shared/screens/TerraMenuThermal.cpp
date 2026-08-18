#include "TerraMenuScreens.h"
#include "TerraMenus.h"
#include "Terraduino.h"
#include "TerraStrings.h"
#include "TerraUIStrings.h"
#include "TerraUIData.h"
#include "TerraUIInlines.hh"

void terraBuildThermalScreen(const Terraduino &controller, TerraMenuModel &model) {
    model.clear("Thermal");
    for(uint8_t i=0;i<controller.objectCount();++i){ TerraObject *o=controller.objectAt(i); if(!o) continue;
        if(o->getObjectType()==Terra_ObjectType_ThermalStore){ const TerraThermalStore *s=static_cast<const TerraThermalStore*>(o); model.add(o->getName(), terraUIFloat(s->getTemperature()) + " C"); }
        else if(o->getObjectType()==Terra_ObjectType_ThermalLoop){ const TerraThermalLoop *l=static_cast<const TerraThermalLoop*>(o); model.add(o->getName(), l->isRunning()?"Running":"Idle"); }
    }
    if(!model.rowCount()) model.add("Status","No thermal objects");
}
