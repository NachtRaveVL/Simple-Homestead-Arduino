// Simple-Homestead-Arduino Weather Station Example
//
// Shows a local environment assembled from normal Terraduino sensor objects.
// Replace the sample measurements with installed sensor drivers as needed.

#include <Terraduino.h>

Terraduino terraController;
SharedPtr<TerraEnvironment> weather;
SharedPtr<TerraRemoteSensor> airTemperature;
SharedPtr<TerraRemoteSensor> humidity;
SharedPtr<TerraRemoteSensor> pressure;
SharedPtr<TerraRemoteSensor> windSpeed;
SharedPtr<TerraRemoteSensor> windDirection;
SharedPtr<TerraRemoteSensor> rainRate;
SharedPtr<TerraRemoteSensor> solarRadiation;

void setup()
{
    Serial.begin(115200);

    terraController.init();
    weather = terraController.addEnvironment("Weather Station");
    airTemperature = terraController.addRemoteSensor(Terra_SensorType_Temperature, Terra_UnitsType_Temperature_Celsius);
    humidity = terraController.addRemoteSensor(Terra_SensorType_Humidity, Terra_UnitsType_Percentile_100);
    pressure = terraController.addRemoteSensor(Terra_SensorType_Pressure, Terra_UnitsType_Pressure_Hectopascals);
    windSpeed = terraController.addRemoteSensor(Terra_SensorType_WindSpeed, Terra_UnitsType_Speed_MetersPerSecond);
    windDirection = terraController.addRemoteSensor(Terra_SensorType_WindDirection, Terra_UnitsType_Angle_Degrees_360);
    rainRate = terraController.addRemoteSensor(Terra_SensorType_Rainfall, Terra_UnitsType_Speed_MillimetersPerHour);
    solarRadiation = terraController.addRemoteSensor(Terra_SensorType_SolarRadiation, Terra_UnitsType_Irradiance_WattsPerSquareMeter);

    weather->setAirTemperatureSensor(airTemperature);
    weather->setHumiditySensor(humidity);
    weather->setPressureSensor(pressure);
    weather->setWindSpeedSensor(windSpeed);
    weather->setWindDirectionSensor(windDirection);
    weather->setRainRateSensor(rainRate);
    weather->setSolarRadiationSensor(solarRadiation);

    terraController.launch();
}

void loop()
{
    // Sample values stand in for sensor-driver readings.
    airTemperature->receiveReport(12.5f, Terra_UnitsType_Temperature_Celsius);
    humidity->receiveReport(73.0f, Terra_UnitsType_Percentile_100);
    pressure->receiveReport(1013.2f, Terra_UnitsType_Pressure_Hectopascals);
    windSpeed->receiveReport(3.2f, Terra_UnitsType_Speed_MetersPerSecond);
    windDirection->receiveReport(245.0f, Terra_UnitsType_Angle_Degrees_360);
    rainRate->receiveReport(1.8f, Terra_UnitsType_Speed_MillimetersPerHour);
    solarRadiation->receiveReport(420.0f, Terra_UnitsType_Irradiance_WattsPerSquareMeter);

    terraController.update();

    Serial.print(F("Dew point C: "));
    Serial.println(weather->dewPointC(), 2);
    Serial.print(F("Freeze risk: "));
    Serial.println(weather->isFreezing() ? F("YES") : F("NO"));
    delay(5000);
}
