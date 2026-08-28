// Simple-Homestead-Arduino Thermal Storage Example
//
// Demonstrates differential circulation between a heat source and a thermal store. The
// thermal loop owns the source/store sensing and circulator control during normal updates.
// Independent hardware temperature and pressure protection remains responsible for safety.

#include <Terraduino.h>

Terraduino terraController;
SharedPtr<TerraSensor> collectorTemperature;
SharedPtr<TerraSensor> storeTemperature;
SharedPtr<TerraThermalReservoir> heatStore;
SharedPtr<TerraThermalLoop> collectorLoop;
SharedPtr<TerraActuator> circulator;

void driveCirculator(void *context, float output)
{
    (void)context;
    Serial.print(F("Circulator output: "));
    Serial.println(output, 2);
}

void setup()
{
    Serial.begin(115200);

    terraController.init();
    collectorTemperature = terraController.addSensor(Terra_SensorType_Temperature, Terra_UnitsType_Celsius, 0, "Collector Temperature");
    storeTemperature = terraController.addSensor(Terra_SensorType_Temperature, Terra_UnitsType_Celsius, 0, "Store Temperature");
    heatStore = terraController.addThermalReservoir(0, "Hot Water Store");
    collectorLoop = terraController.addThermalLoop(0, "Collector Loop");
    circulator = terraController.addCirculator(0, "Collector Circulator");

    heatStore->setTargetRange(45.0f, 65.0f);
    heatStore->setAbsoluteMaximum(90.0f);
    heatStore->setTemperatureSensor(storeTemperature);

    collectorLoop->configure(8.0f, 3.0f, 80.0f);
    collectorLoop->setSourceTemperatureSensor(collectorTemperature);
    collectorLoop->setThermalReservoir(heatStore);
    collectorLoop->setCirculator(circulator);

    circulator->setDriver(SharedPtr<TerraOutputDriver>(new TerraCallbackOutputDriver(driveCirculator)));
    terraController.launch();
}

void loop()
{
    // Replace these values with installed temperature sensor drivers.
    collectorTemperature->setMeasurement(72.0f, Terra_UnitsType_Celsius, terraMillis(), true);
    storeTemperature->setMeasurement(52.0f, Terra_UnitsType_Celsius, terraMillis(), true);

    terraController.update();

    if (heatStore->isSafetyLimitExceeded()) {
        Serial.println(F("Thermal store safety limit exceeded"));
    }

    delay(1000);
}
