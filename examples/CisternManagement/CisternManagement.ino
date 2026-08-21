// Simple-Homestead-Arduino Cistern Management Example
//
// Demonstrates fill hysteresis, protected reserve, and pump control for a primary cistern.
// The pump driver is intentionally generic so the real installation can use an isolated
// relay, motor controller, or another suitable low-voltage interface.

#include <Terraduino.h>

Terraduino terraController;
SharedPtr<TerraWaterSource> well;
SharedPtr<TerraCistern> cistern;
SharedPtr<TerraWaterRoute> fillRoute;
SharedPtr<TerraPump> fillPump;
SharedPtr<TerraCallbackOutputDriver> fillPumpDriver;

void driveFillPump(void *context, float output)
{
    (void)context;
    Serial.print(F("Fill pump output: "));
    Serial.println(output, 2);
    // Drive the real isolated pump-control interface here.
}

void setup()
{
    Serial.begin(115200);

    terraController.init();
    well = terraController.addWaterSource(Terra_WaterSourceType_Well, 0, 0, "Well");
    cistern = terraController.addCistern(5000.0f, 0, "Main Cistern");
    fillRoute = terraController.addWaterRoute(0, "Cistern Fill");
    fillPump = terraController.addPump(0, "Fill Pump");
    fillPumpDriver = SharedPtr<TerraCallbackOutputDriver>(new TerraCallbackOutputDriver(driveFillPump));

    well->setLevel(100.0f);
    well->setReserveLevel(10.0f);
    well->setMaximumFlowLpm(20.0f);

    cistern->setThresholds(15.0f, 30.0f, 95.0f);
    cistern->configureFillBand(30.0f, 90.0f, 99.0f);
    cistern->setLevel(28.0f);

    fillPump->setMaxContinuousRuntime(15UL * 60UL * 1000UL);
    fillPump->setDriver(fillPumpDriver);

    fillRoute->setSource(well);
    fillRoute->setDestination(cistern);
    fillRoute->setPump(fillPump);
    fillRoute->setDestinationBand(30.0f, 90.0f);
    fillRoute->setMaximumFlow(20.0f);

    terraController.launch();
}

void loop()
{
    // The route owns fill hysteresis and pump control during controller updates.
    terraController.update();
    delay(1000);
}
