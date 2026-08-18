// Simple-Homestead-Arduino Thermal Storage Example
//
// Demonstrates differential circulation between a heat source and a thermal store. The
// library decides when circulation is useful, while independent hardware temperature and
// pressure protection remains responsible for installation safety.

#include <Terraduino.h>

TerraThermalStore heatStore(0, "Hot Water Store");
TerraThermalLoop collectorLoop(0, "Collector Loop");
TerraThermalBalancer thermalBalancer;

void setup()
{
    Serial.begin(115200);

    heatStore.setTargetRange(45.0f, 65.0f);
    heatStore.setAbsoluteMaximum(90.0f);
    collectorLoop.configure(8.0f, 3.0f, 80.0f);
}

void loop()
{
    // Replace these values with collector and storage temperature sensors.
    float collectorTemperatureC = 72.0f;
    float storeTemperatureC = 52.0f;
    heatStore.setTemperature(storeTemperatureC);

    bool circulate = thermalBalancer.evaluate(collectorLoop, collectorTemperatureC, storeTemperatureC);
    if (circulate) {
        // Command a suitable external circulation-pump controller here.
    }

    if (heatStore.isSafetyLimitExceeded()) {
        Serial.println(F("Thermal store safety limit exceeded"));
    }

    delay(1000);
}
