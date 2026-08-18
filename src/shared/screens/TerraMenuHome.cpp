#include "TerraMenuScreens.h"
#include "TerraMenus.h"
#include "Terraduino.h"
#include "TerraStrings.h"
#include "TerraUIStrings.h"
#include "TerraUIData.h"
#include "TerraUIInlines.hh"

void terraBuildHomeScreen(const Terraduino &controller, TerraMenuModel &model) {
    TerraUIData data; data.capture(controller); model.clear("Home");
    model.add("System", data.systemName); model.add("Running", terraUIBool(data.running));
    model.add("Objects", terraUINumber(data.objectCount)); model.add("Faults", terraUINumber(data.faultCount));
    model.add("Low water", terraUINumber(data.lowWaterCount)); model.add("Freeze risk", terraUIBool(data.freezeRisk));
}
