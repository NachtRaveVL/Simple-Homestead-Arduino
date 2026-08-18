#include "TerraMenuScreens.h"
#include "TerraMenus.h"
#include "Terraduino.h"
#include "TerraStrings.h"
#include "TerraUIStrings.h"
#include "TerraUIData.h"
#include "TerraUIInlines.hh"

void terraBuildSchedulingScreen(const Terraduino &controller, TerraMenuModel &model) {
    model.clear("Scheduling"); model.add("Controller",controller.isRunning()?"Running":"Stopped"); model.add("Update ms",terraUINumber(controller.getSetup().updateIntervalMs));
}
