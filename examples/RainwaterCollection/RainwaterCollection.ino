// Simple-Homestead-Arduino Rainwater Collection Example
//
// Converts incremental roof rainfall into estimated catchment volume and applies a
// first-flush discard. Application code can route the remaining captured water through
// the same reservoir/pump plumbing used elsewhere in the system.

#include <Terraduino.h>

Terraduino terraController;
TerraFirstFlushController firstFlush(20.0f);

void setup()
{
    Serial.begin(115200);

    terraController.init();
    terraController.launch();
}

void loop()
{
    // Replace with incremental rainfall from the installed rain gauge.
    const float rainfallMm = 0.0f;
    float capturedLiters = roofCatchment->estimateCaptureLiters(rainfallMm);
    float discardedLiters = 0.0f;

    if (firstFlush.shouldDivert()) {
        discardedLiters = min(capturedLiters, firstFlush.getRemainingLiters());
        firstFlush.recordFlow(discardedLiters);
    }

    float availableLiters = capturedLiters - discardedLiters;
    if (availableLiters > 0.0f) {
        Serial.print(F("Captured rainwater after first flush, L: "));
        Serial.println(availableLiters, 2);
    }

    terraController.update();
    delay(1000);
}
