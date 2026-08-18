#include <Terraduino.h>

void setup() {
    Serial.begin(115200);
    while (!Serial) { }

    TerraCisternData cistern;
    cistern.key = 1001;
    cistern.name = "Main Cistern";
    cistern.capacityLiters = 5000.0f;
    cistern.level = 63.5f;
    cistern.reserveLevel = 15.0f;
    cistern.lowLevel = 30.0f;
    cistern.highLevel = 95.0f;
    cistern.fillStartPercent = 35.0f;
    cistern.fillStopPercent = 90.0f;
    cistern.overflowPercent = 99.0f;

    Serial.println(cistern.toJSON());

    TerraThermalStoreData thermal;
    thermal.key = 2001;
    thermal.name = "Thermal Store";
    thermal.level = 70.0f;
    thermal.temperatureC = 58.0f;
    thermal.minimumTargetC = 45.0f;
    thermal.maximumTargetC = 65.0f;
    thermal.absoluteMaximumC = 90.0f;

    Serial.println(thermal.toJSON());
}

void loop() { }
