// Simple-Homestead-Arduino Remote Sensor Example
//
// Demonstrates the transport-neutral remote sensor object. MQTT, ESP-NOW, LoRa, RS-485,
// CAN, serial, or another link can feed the same object without making networking a
// requirement for the main Terraduino controller.

#include <Terraduino.h>

TerraRemoteSensor barnTemperature(Terra_SensorType_Temperature,
                                  Terra_Unit_Celsius,
                                  0,
                                  "Barn Temperature");

void onRemoteMessage(float temperatureC)
{
    barnTemperature.receiveReport(temperatureC, Terra_Unit_Celsius, millis(), true);
}

void setup()
{
    Serial.begin(115200);
    barnTemperature.setStaleAfter(5UL * 60UL * 1000UL);
}

void loop()
{
    // Call onRemoteMessage(value) from the selected transport handler.
    barnTemperature.update(millis());

    if (!barnTemperature.isOnline(millis())) {
        Serial.println(F("Remote barn sensor is stale or offline"));
    }

    delay(1000);
}
