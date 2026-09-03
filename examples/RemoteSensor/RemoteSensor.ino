// Simple-Homestead-Arduino Remote Sensor Example
//
// Demonstrates the transport-neutral remote sensor object. MQTT, ESP-NOW, LoRa, RS-485,
// CAN, serial, or another link can feed the same object without making networking a
// requirement for the main Terraduino controller.

#include <Terraduino.h>

Terraduino terraController;
SharedPtr<TerraRemoteSensor> barnTemperature;

void onRemoteMessage(float temperatureC)
{
    if (barnTemperature) {
        barnTemperature->receiveReport(temperatureC, Terra_UnitsType_Temperature_Celsius, millis(), true);
    }
}

void setup()
{
    Serial.begin(115200);
    terraController.init();
    barnTemperature = terraController.addRemoteSensor(Terra_SensorType_Temperature,
                                                       Terra_UnitsType_Temperature_Celsius);
    if (barnTemperature) { barnTemperature->setStaleAfter(5UL * 60UL * 1000UL); }
    terraController.launch();
}

void loop()
{
    // Call onRemoteMessage(value) from the selected transport handler.
    terraController.update();

    if (barnTemperature && !barnTemperature->isOnline(millis())) {
        Serial.println(F("Remote barn sensor is stale or offline"));
    }

    delay(1000);
}
