// Simple-Homestead-Arduino Basic Homestead Example
//
// Shows the smallest useful Terraduino setup with local weather, a cistern, and a thermal
// store registered under one controller. Real sensors and actuators can be attached later
// without changing the basic controller lifecycle.

#include <Terraduino.h>

Terraduino terraController;
SharedPtr<TerraEnvironment> outside;
SharedPtr<TerraSensor> outsideTemperature;
SharedPtr<TerraSensor> outsideHumidity;
SharedPtr<TerraCistern> cistern;
SharedPtr<TerraSensor> cisternLevel;
SharedPtr<TerraThermalStore> thermalTank;
SharedPtr<TerraSensor> thermalTankTemperature;

void setup()
{
    Serial.begin(115200);

    terraController.init(Terra_ControlMode_Automatic, Terra_MeasurementMode_Metric);
    terraController.setSystemName("Homestead");
    outside = terraController.addEnvironment("Outside");
    outsideTemperature = terraController.addSensor(Terra_SensorType_Temperature, Terra_Unit_Celsius, "Outside Temperature");
    outsideHumidity = terraController.addSensor(Terra_SensorType_Humidity, Terra_Unit_Percent, "Outside Humidity");
    cistern = terraController.addCistern(2000.0f, "Main Cistern");
    cisternLevel = terraController.addSensor(Terra_SensorType_Level, Terra_Unit_Percent, "Cistern Level");
    thermalTank = terraController.addThermalStore("Thermal Store");
    thermalTankTemperature = terraController.addSensor(Terra_SensorType_Temperature, Terra_Unit_Celsius, "Thermal Store Temperature");

    outside->setAirTemperatureSensor(outsideTemperature);
    outside->setHumiditySensor(outsideHumidity);
    cistern->setLevelSensor(cisternLevel);
    thermalTank->setTemperatureSensor(thermalTankTemperature);

    cistern->setThresholds(20.0f, 35.0f, 90.0f);
    cistern->configureFillBand(35.0f, 90.0f, 99.0f);
    thermalTank->setTargetRange(45.0f, 65.0f);
    thermalTank->setAbsoluteMaximum(90.0f);

    terraController.launch();
}

void loop()
{
    // Replace these sample sensor updates with installed sensor hardware.
    outsideTemperature->setMeasurement(12.0f, Terra_Unit_Celsius, millis(), true);
    outsideHumidity->setMeasurement(65.0f, Terra_Unit_Percent, millis(), true);
    cisternLevel->setMeasurement(60.0f, Terra_Unit_Percent, millis(), true);
    thermalTankTemperature->setMeasurement(54.0f, Terra_Unit_Celsius, millis(), true);

    terraController.update();
    delay(1000);
}
