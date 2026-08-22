// Simple-Homestead-Arduino Data Writer Example
//
// Builds a small controller configuration and exports it through the same TerraData /
// ArduinoJson persistence path used by normal controller saves.

#include <Terraduino.h>

Terraduino terraController;

void setup()
{
    Serial.begin(115200);
    while (!Serial) { ; }

    terraController.init();

    auto cistern = terraController.addCistern(5000.0f, "Main Cistern");
    cistern->setThresholds(15.0f, 30.0f, 95.0f);
    cistern->configureFillBand(35.0f, 90.0f, 99.0f);
    cistern->setLevel(63.5f);

    auto thermal = terraController.addThermalStore("Thermal Store");
    thermal->setLevel(70.0f);
    thermal->setTemperature(58.0f);
    thermal->setTargetRange(45.0f, 65.0f);
    thermal->setAbsoluteMaximum(90.0f);

    Serial.println(terraController.exportSystemJSON());
}

void loop() { ; }
