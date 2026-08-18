// Simple-Homestead-Arduino Cistern Management Example
//
// Demonstrates fill hysteresis, protected reserve, source selection, and pump control for
// a primary cistern. The pump callback is intentionally generic so the real installation
// can use an isolated relay, motor controller, or another suitable low-voltage interface.

#include <Terraduino.h>

Terraduino terraController;
TerraWaterSource well(Terra_WaterSourceType_Well, 0, 0, "Well");
TerraCistern cistern(5000.0f, 0, "Main Cistern");
TerraWaterRoute fillRoute(0, "Cistern Fill");
TerraPump fillPump(0, "Fill Pump");
TerraWaterBalancer waterBalancer;

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
    terraController.registerObject(&well);
    terraController.registerObject(&cistern);
    terraController.registerObject(&fillRoute);
    terraController.registerObject(&fillPump);

    well.setPriority(0);
    well.setLevel(100.0f);
    well.setReserveLevel(10.0f);
    well.setMaximumFlowLpm(20.0f);

    cistern.setThresholds(15.0f, 30.0f, 95.0f);
    cistern.configureFillBand(30.0f, 90.0f, 99.0f);
    cistern.setLevel(28.0f);

    fillRoute.configure(well.getKey(), cistern.getKey(), 30.0f, 90.0f);
    fillRoute.setMinimumFlow(0.5f);
    fillRoute.setMaximumFlow(20.0f);

    fillPump.setMaxContinuousRuntime(15UL * 60UL * 1000UL);
    fillPump.setWriteCallback(driveFillPump);
    terraController.launch();
}

void loop()
{
    TerraTransferDecision decision = waterBalancer.evaluate(fillRoute, well, cistern);

    if (decision.shouldRun) {
        fillPump.setOutput(1.0f);
    } else {
        fillPump.off();
    }

    terraController.update();
    delay(1000);
}
