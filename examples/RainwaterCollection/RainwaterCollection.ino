// Simple-Homestead-Arduino Rainwater Collection Example
//
// Converts incremental roof rainfall into cistern inflow, applies a first-flush discard,
// and respects the configured cistern overflow band. A real rain gauge should supply the
// rainfall accumulated since the previous collection update.

#include <Terraduino.h>

Terraduino terraController;
TerraRainCatchment roofCatchment(180.0f, 0.85f, 0, "Roof Catchment");
TerraCistern rainCistern(5000.0f, 0, "Rain Cistern");
TerraFirstFlushController firstFlush(20.0f);

void setup()
{
    Serial.begin(115200);

    terraController.init();
    terraController.registerObject(&roofCatchment);
    terraController.registerObject(&rainCistern);

    rainCistern.setThresholds(15.0f, 30.0f, 95.0f);
    rainCistern.configureFillBand(30.0f, 95.0f, 99.0f);
    rainCistern.setLevel(25.0f);
    terraController.launch();
}

void loop()
{
    // Replace with incremental rainfall from the installed rain gauge.
    const float rainfallMm = 0.0f;
    TerraRainCollectionResult result = roofCatchment.collectInto(rainCistern, rainfallMm, &firstFlush);

    if (result.storedLiters > 0.0f) {
        Serial.print(F("Stored rainwater, L: "));
        Serial.println(result.storedLiters, 2);
    }

    terraController.update();
    delay(1000);
}
