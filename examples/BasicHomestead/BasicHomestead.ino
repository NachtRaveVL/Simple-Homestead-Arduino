// Simple-Homestead-Arduino Basic Homestead Example
//
// Shows the smallest useful Terraduino setup with remote environmental measurements, a
// water reservoir, and a thermal reservoir registered under one controller. Real sensors
// and actuators can be attached later without changing the basic controller lifecycle.

#include <Terraduino.h>

Terraduino terraController;
SharedPtr<TerraRemoteSensor> outsideTemperature;
SharedPtr<TerraRemoteSensor> outsideHumidity;
SharedPtr<TerraWaterReservoir> waterTank;
SharedPtr<TerraRemoteSensor> waterVolume;
SharedPtr<TerraThermalReservoir> thermalTank;
SharedPtr<TerraRemoteSensor> thermalTankTemperature;

void setup()
{
    Serial.begin(115200);

    terraController.init(Terra_SystemMode_Automatic, Terra_MeasurementMode_Metric);
    terraController.setSystemName("Homestead");
    outsideTemperature = terraController.addRemoteSensor(Terra_SensorType_Temperature, Terra_UnitsType_Temperature_Celsius);
    outsideHumidity = terraController.addRemoteSensor(Terra_SensorType_Humidity, Terra_UnitsType_Percentile_100);
    waterTank = terraController.addWaterReservoir(2000.0f);
    waterVolume = terraController.addRemoteSensor(Terra_SensorType_Level, Terra_UnitsType_LiqVolume_Liters);
    thermalTank = terraController.addThermalReservoir(90.0f);
    thermalTankTemperature = terraController.addRemoteSensor(Terra_SensorType_Temperature, Terra_UnitsType_Temperature_Celsius);

    waterTank->getWaterVolumeSensorAttachment().setObject(waterVolume);
    thermalTank->getMediumTemperatureSensorAttachment().setObject(thermalTankTemperature);

    terraController.launch();
}

void loop()
{
    // Replace these sample reports with installed sensor hardware.
    outsideTemperature->receiveReport(12.0f, Terra_UnitsType_Temperature_Celsius);
    outsideHumidity->receiveReport(65.0f, Terra_UnitsType_Percentile_100);
    waterVolume->receiveReport(1200.0f, Terra_UnitsType_LiqVolume_Liters);
    thermalTankTemperature->receiveReport(54.0f, Terra_UnitsType_Temperature_Celsius);

    terraController.update();
    delay(1000);
}
