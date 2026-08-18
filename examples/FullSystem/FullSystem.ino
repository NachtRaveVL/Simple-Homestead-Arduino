// Simple-Homestead-Arduino Full System Example
//
// Combines weather observations, source selection, cistern filling, thermal storage, a
// remote sensor, logging/publishing, and normal controller updates. The final hardware
// adapters remain application supplied so the example is not tied to one shield or board.

#include <Terraduino.h>

Terraduino terraController;
TerraEnvironment weather(0, "Outside");
TerraWaterSource rain(Terra_WaterSourceType_Rainwater, 0, 0, "Rainwater");
TerraWaterSource well(Terra_WaterSourceType_Well, 1, 0, "Well");
TerraCistern cistern(4000.0f, 0, "Cistern");
TerraWaterRoute fillRoute(0, "Cistern Fill");
TerraPump transferPump(0, "Transfer Pump");
TerraThermalStore thermalStore(0, "Thermal Store");
TerraThermalLoop thermalLoop(0, "Thermal Loop");
TerraRemoteSensor barnTemp(Terra_SensorType_Temperature, Terra_Unit_Celsius, 0, "Barn Temp");
TerraWaterBalancer waterBalancer;
TerraThermalBalancer thermalBalancer;

void actuatorWrite(void *context, float value)
{
    (void)context;
    Serial.print(F("Pump command: "));
    Serial.println(value, 2);
    // Replace with a suitable isolated driver for the real load.
}

void publishValue(void *context, const char *channel, const TerraMeasurement &measurement)
{
    (void)context;
    Serial.print(channel);
    Serial.print('=');
    Serial.print(measurement.value);
    Serial.print(' ');
    Serial.println(terraUnitToString(measurement.unit));
}

void setup()
{
    Serial.begin(115200);

    TerraSystemSetup setup;
    setup.systemName = "Full Homestead";
    setup.updateIntervalMs = 250;
    terraController.init(setup);

    terraController.registerObject(&weather);
    terraController.registerObject(&rain);
    terraController.registerObject(&well);
    terraController.registerObject(&cistern);
    terraController.registerObject(&fillRoute);
    terraController.registerObject(&transferPump);
    terraController.registerObject(&thermalStore);
    terraController.registerObject(&thermalLoop);
    terraController.registerObject(&barnTemp);

    cistern.setThresholds(15.0f, 30.0f, 95.0f);
    cistern.configureFillBand(30.0f, 95.0f, 99.0f);
    cistern.setLevel(25.0f);
    rain.setLevel(80.0f);
    well.setLevel(100.0f);
    fillRoute.configure(rain.getKey(), cistern.getKey(), 30.0f, 95.0f);
    fillRoute.setMinimumFlow(0.5f);
    transferPump.setMaxContinuousRuntime(10UL * 60UL * 1000UL);
    transferPump.setWriteCallback(actuatorWrite);

    thermalStore.setTargetRange(45.0f, 65.0f);
    thermalStore.setAbsoluteMaximum(90.0f);
    thermalLoop.configure(8.0f, 3.0f, 80.0f);

    barnTemp.setStaleAfter(5UL * 60UL * 1000UL);
    terraController.publisher.setCallback(publishValue);
    terraController.publisher.addChannel("barn-temp", &barnTemp, Terra_Unit_Celsius);
    terraController.launch();
}

void loop()
{
    weather.setAirTemperature(8.0f);
    weather.setRelativeHumidity(78.0f);

    const TerraWaterSource *sources[] = { &rain, &well };
    const TerraWaterSource *selected = waterBalancer.selectSource(sources, 2);

    if (selected) {
        fillRoute.configure(selected->getKey(), cistern.getKey(), 30.0f, 95.0f);
        TerraTransferDecision decision = waterBalancer.evaluate(fillRoute, *selected, cistern);
        if (decision.shouldRun) { transferPump.setOutput(1.0f); }
        else { transferPump.off(); }
    } else {
        transferPump.off();
    }

    thermalStore.setTemperature(52.0f);
    thermalBalancer.evaluate(thermalLoop, 70.0f, thermalStore.getTemperature());

    terraController.update();
    delay(250);
}
