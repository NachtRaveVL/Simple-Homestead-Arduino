#include "TerraMenuScreens.h"
#include "TerraMenus.h"
#include "Terraduino.h"
#include "TerraStrings.h"
#include "TerraUIStrings.h"
#include "TerraUIData.h"
#include "TerraUIInlines.hh"

void terraBuildAlertsScreen(const Terraduino &controller, TerraMenuModel &model) {
    model.clear("Alerts"); for(uint8_t i=0;i<controller.objectCount();++i){ TerraObject *o=controller.objectAt(i); if(o&&o->hasFault()) model.add(o->getName(),o->getFaultMessage()); }
    if(!model.rowCount()) model.add("Status","No active faults");
}
