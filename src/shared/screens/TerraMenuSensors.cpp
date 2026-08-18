#include "TerraMenuScreens.h"
#include "TerraMenus.h"
#include "Terraduino.h"
#include "TerraStrings.h"
#include "TerraUIStrings.h"
#include "TerraUIData.h"
#include "TerraUIInlines.hh"

void terraBuildSensorsScreen(const Terraduino &controller, TerraMenuModel &model) {
    model.clear("Sensors"); for(uint8_t i=0;i<controller.objectCount();++i){ TerraObject *o=controller.objectAt(i); if(!o||o->getObjectType()!=Terra_ObjectType_Sensor) continue;
        const TerraSensor *s=static_cast<const TerraSensor*>(o); const TerraMeasurement m=s->getMeasurement(); model.add(o->getName(), m.valid ? terraUIFloat(m.value)+" "+terraUnitToString(m.unit) : "No data"); }
    if(!model.rowCount()) model.add("Status","No sensors");
}
