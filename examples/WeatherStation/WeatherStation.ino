// Simple-Homestead-Arduino Weather Station Example
//
// Shows the environmental snapshot used by Terraduino for local logging and control.
// Individual values can come from any supported sensor or user-provided adapter.

#include <Terraduino.h>

TerraEnvironment weather(0, "Weather Station");

void setup()
{
    Serial.begin(115200);
}

void loop()
{
    // Replace these sample values with installed sensors.
    weather.setAirTemperature(12.5f);
    weather.setRelativeHumidity(73.0f);
    weather.setBarometricPressure(1013.2f);
    weather.setWind(3.2f, 245.0f);
    weather.setRainfallRate(1.8f);
    weather.setSolarRadiation(420.0f);

    Serial.print(F("Dew point C: "));
    Serial.println(weather.dewPointC(), 2);
    Serial.print(F("Freeze risk: "));
    Serial.println(weather.isFreezing() ? F("YES") : F("NO"));
    delay(5000);
}
