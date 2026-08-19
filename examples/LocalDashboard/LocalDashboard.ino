// Simple-Homestead-Arduino Local Dashboard Example
//
// Keeps a small local status display on the serial console without requiring networking
// or the project-specific tcMenu screens, which are still under development.

#include <Terraduino.h>

Terraduino terraController;
TerraCistern cistern(5000.0f, 1001, "Main Cistern");
TerraEnvironment outside(1002, "Outside");
uint32_t lastReport = 0;

void setup()
{
    Serial.begin(115200);

    terraController.init();
    cistern.setThresholds(15.0f, 30.0f, 95.0f);
    cistern.configureFillBand(35.0f, 90.0f, 99.0f);
    cistern.setStoredLiters(3200.0f);
    outside.setAirTemperature(4.0f);

    terraController.registerObject(&cistern);
    terraController.registerObject(&outside);
    terraController.launch();
}

void loop()
{
    terraController.update();

    uint32_t now = terraMillis();
    if (!lastReport || terraElapsed(now, lastReport, 5000)) {
        Serial.print(F("Cistern: "));
        Serial.print(cistern.getLevel(), 1);
        Serial.print(F("%  Outside: "));
        Serial.print(outside.getAirTemperature(), 1);
        Serial.println(F(" C"));
        lastReport = now;
    }
}
