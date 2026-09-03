// Simple-Homestead-Arduino Weather Station Example
//
// Demonstrates Terraduino as a networked sensor station. The controller collects normal
// sensor measurements and its existing Publisher sends them to an MQTT broker. Enable
// TERRA_ENABLE_WIFI and TERRA_ENABLE_MQTT through build flags or Terraduino.h to enable
// the network portion of this example.

#include <Terraduino.h>

#define SETUP_WIFI_SSID                 "CHANGE_ME"
#define SETUP_WIFI_PASS                 "CHANGE_ME"
#define SETUP_MQTT_BROKER               "broker.example"
#define SETUP_MQTT_PORT                 1883

Terraduino terraController;

#if defined(TERRA_USE_WIFI) && defined(TERRA_USE_MQTT)
WiFiClient netClient;
MQTTClient mqttClient;
#endif

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

    terraController.init(Terra_SystemMode_Automatic, Terra_MeasurementMode_Metric);
    terraController.setSystemName("Weather Station");
    terraController.setPollingInterval(5000);

    airTemperature = terraController.addRemoteSensor(Terra_SensorType_Temperature,
                                                       Terra_UnitsType_Temperature_Celsius);
    humidity = terraController.addRemoteSensor(Terra_SensorType_Humidity,
                                                Terra_UnitsType_Percentile_100);
    pressure = terraController.addRemoteSensor(Terra_SensorType_Pressure,
                                                Terra_UnitsType_Pressure_Hectopascals);
    windSpeed = terraController.addRemoteSensor(Terra_SensorType_WindSpeed,
                                                 Terra_UnitsType_Speed_MetersPerSecond);
    windDirection = terraController.addRemoteSensor(Terra_SensorType_WindDirection,
                                                     Terra_UnitsType_Angle_Degrees_360);
    rainRate = terraController.addRemoteSensor(Terra_SensorType_Rainfall,
                                                Terra_UnitsType_Speed_MillimetersPerHour);
    solarRadiation = terraController.addRemoteSensor(Terra_SensorType_SolarRadiation,
                                                      Terra_UnitsType_Irradiance_WattsPerSquareMeter);

    #if defined(TERRA_USE_WIFI) && defined(TERRA_USE_MQTT)
        terraController.setWiFiConnection(SETUP_WIFI_SSID, SETUP_WIFI_PASS);
        if (terraController.getWiFi()) {
            mqttClient.begin(SETUP_MQTT_BROKER, SETUP_MQTT_PORT, netClient);
            terraController.enableDataPublishingToMQTTClient(mqttClient);
        }
    #else
        Serial.println(F("Enable TERRA_ENABLE_WIFI and TERRA_ENABLE_MQTT for MQTT publishing."));
    #endif

    terraController.launch();
}

void loop()
{
    #if defined(TERRA_USE_WIFI) && defined(TERRA_USE_MQTT)
        mqttClient.loop();
    #endif

    // Replace these sample reports with installed weather sensor drivers.
    airTemperature->receiveReport(12.5f, Terra_UnitsType_Temperature_Celsius);
    humidity->receiveReport(73.0f, Terra_UnitsType_Percentile_100);
    pressure->receiveReport(1013.2f, Terra_UnitsType_Pressure_Hectopascals);
    windSpeed->receiveReport(3.2f, Terra_UnitsType_Speed_MetersPerSecond);
    windDirection->receiveReport(245.0f, Terra_UnitsType_Angle_Degrees_360);
    rainRate->receiveReport(1.8f, Terra_UnitsType_Speed_MillimetersPerHour);
    solarRadiation->receiveReport(420.0f, Terra_UnitsType_Irradiance_WattsPerSquareMeter);

    terraController.update();
    delay(5000);
}
