// Simple-Homestead-Arduino Weather Station Example
//
// Shows a local environment assembled from normal Terraduino sensor objects.
// Replace the sample measurements with installed sensor drivers as needed.

#include <Terraduino.h>

Terraduino terraController;
SharedPtr<TerraEnvironment> weather;
SharedPtr<TerraSensor> airTemperature;
SharedPtr<TerraSensor> humidity;
SharedPtr<TerraSensor> pressure;
SharedPtr<TerraSensor> windSpeed;
SharedPtr<TerraSensor> windDirection;
SharedPtr<TerraSensor> rainRate;
SharedPtr<TerraSensor> solarRadiation;

void setup()
{
    Serial.begin(115200);

    terraController.init();
    weather = terraController.addEnvironment(TERRA_INVALID_KEY, "Weather Station");
    airTemperature = terraController.addSensor(Terra_SensorType_Temperature, Terra_UnitsType_Celsius);
    humidity = terraController.addSensor(Terra_SensorType_Humidity, Terra_UnitsType_Percent);
    pressure = terraController.addSensor(Terra_SensorType_Pressure, Terra_UnitsType_Hectopascals);
    windSpeed = terraController.addSensor(Terra_SensorType_WindSpeed, Terra_UnitsType_MetersPerSecond);
    windDirection = terraController.addSensor(Terra_SensorType_WindDirection, Terra_UnitsType_Degrees);
    rainRate = terraController.addSensor(Terra_SensorType_Rainfall, Terra_UnitsType_MillimetersPerHour);
    solarRadiation = terraController.addSensor(Terra_SensorType_SolarRadiation, Terra_UnitsType_WattsPerSquareMeter);

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
    airTemperature->setMeasurement(12.5f, Terra_UnitsType_Celsius);
    humidity->setMeasurement(73.0f, Terra_UnitsType_Percent);
    pressure->setMeasurement(1013.2f, Terra_UnitsType_Hectopascals);
    windSpeed->setMeasurement(3.2f, Terra_UnitsType_MetersPerSecond);
    windDirection->setMeasurement(245.0f, Terra_UnitsType_Degrees);
    rainRate->setMeasurement(1.8f, Terra_UnitsType_MillimetersPerHour);
    solarRadiation->setMeasurement(420.0f, Terra_UnitsType_WattsPerSquareMeter);

    terraController.update();

    Serial.print(F("Dew point C: "));
    Serial.println(weather->dewPointC(), 2);
    Serial.print(F("Freeze risk: "));
    Serial.println(weather->isFreezing() ? F("YES") : F("NO"));
    delay(5000);
}
