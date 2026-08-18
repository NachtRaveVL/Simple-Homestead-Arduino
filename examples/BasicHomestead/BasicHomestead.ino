// Simple-Homestead-Arduino Basic Homestead Example
//
// Shows the smallest useful Terraduino setup with local weather, a cistern, and a thermal
// store registered under one controller. Real sensors and actuators can be attached later
// without changing the basic controller lifecycle.

#include <Terraduino.h>

Terraduino terraController;
TerraEnvironment outside(0, "Outside");
TerraCistern cistern(2000.0f, 0, "Main Cistern");
TerraThermalStore thermalTank(0, "Thermal Store");

void setup()
{
    Serial.begin(115200);

    TerraSystemSetup setup;
    setup.systemName = "Homestead";
    setup.controlMode = Terra_ControlMode_Automatic;

    terraController.init(setup);
    cistern.setThresholds(20.0f, 35.0f, 90.0f);
    cistern.configureFillBand(35.0f, 90.0f, 99.0f);
    cistern.setStoredLiters(1200.0f);
    thermalTank.setTargetRange(45.0f, 65.0f);
    thermalTank.setAbsoluteMaximum(90.0f);

    terraController.registerObject(&outside);
    terraController.registerObject(&cistern);
    terraController.registerObject(&thermalTank);
    terraController.launch();
}

void loop()
{
    // Replace these values with installed weather and storage sensors.
    outside.setAirTemperature(12.0f);
    outside.setRelativeHumidity(65.0f);
    thermalTank.setTemperature(54.0f);

    terraController.update();
    delay(1000);
}
