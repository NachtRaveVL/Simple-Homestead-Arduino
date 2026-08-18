#include "TerraMenuScreens.h"
#include "TerraMenus.h"
#include "Terraduino.h"
#include "TerraStrings.h"
#include "TerraUIStrings.h"
#include "TerraUIData.h"
#include "TerraUIInlines.hh"

void terraBuildSettingsScreen(const Terraduino &controller, TerraMenuModel &model) {
    model.clear("Settings"); model.add("System",controller.getSetup().systemName); model.add("Control",terraControlModeToString(controller.getSetup().controlMode));
    model.add("Update ms",terraUINumber(controller.getSetup().updateIntervalMs)); model.add("Timezone hr",terraUINumber(controller.getSetup().timeZoneHours));
}
