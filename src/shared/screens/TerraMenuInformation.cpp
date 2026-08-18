#include "TerraMenuScreens.h"
#include "TerraMenus.h"
#include "Terraduino.h"
#include "TerraStrings.h"
#include "TerraUIStrings.h"
#include "TerraUIData.h"
#include "TerraUIInlines.hh"

void terraBuildInformationScreen(const Terraduino &controller, TerraMenuModel &model) {
    TerraUIData data; data.capture(controller); model.clear("Information"); model.add("Library","Terraduino"); model.add("Objects",terraUINumber(data.objectCount));
    model.add("Sensors",terraUINumber(data.sensorCount)); model.add("Actuators",terraUINumber(data.actuatorCount)); model.add("Water stores",terraUINumber(data.waterStorageCount)); model.add("Thermal stores",terraUINumber(data.thermalStoreCount));
}
