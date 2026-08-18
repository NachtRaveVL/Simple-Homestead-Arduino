#include "TerraMenuScreens.h"
#include "TerraMenus.h"
#include "Terraduino.h"
#include "TerraStrings.h"
#include "TerraUIStrings.h"
#include "TerraUIData.h"
#include "TerraUIInlines.hh"

void terraBuildActuatorsScreen(const Terraduino &controller, TerraMenuModel &model) {
    model.clear("Actuators"); for(uint8_t i=0;i<controller.objectCount();++i){ TerraObject *o=controller.objectAt(i); if(!o||o->getObjectType()!=Terra_ObjectType_Actuator) continue;
        const TerraActuator *a=static_cast<const TerraActuator*>(o); model.add(o->getName(), a->isActive()?terraUIFloat(a->getOutput()*100.0f)+"%":"Off"); }
    if(!model.rowCount()) model.add("Status","No actuators");
}
