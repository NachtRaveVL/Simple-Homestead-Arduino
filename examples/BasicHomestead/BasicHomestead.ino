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

    TerraSystemSetup setup;
    setup.systemName = "Homestead";
    setup.controlMode = Terra_ControlMode_Automatic;

    terraController.init(setup);
    outside = terraController.addEnvironment(0, "Outside");
    outsideTemperature = terraController.addSensor(Terra_SensorType_Temperature, Terra_Unit_Celsius, 0, "Outside Temperature");
    outsideHumidity = terraController.addSensor(Terra_SensorType_Humidity, Terra_Unit_Percent, 0, "Outside Humidity");
    cistern = terraController.addCistern(2000.0f, 0, "Main Cistern");
    cisternLevel = terraController.addSensor(Terra_SensorType_Level, Terra_Unit_Percent, 0, "Cistern Level");
    thermalTank = terraController.addThermalStore(0, "Thermal Store");
    thermalTankTemperature = terraController.addSensor(Terra_SensorType_Temperature, Terra_Unit_Celsius, 0, "Thermal Store Temperature");

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
    // Replace these sample sensor updates with installed sensor drivers.
    outsideTemperature->setMeasurement(12.0f, Terra_Unit_Celsius, terraMillis(), true);
    outsideHumidity->setMeasurement(65.0f, Terra_Unit_Percent, terraMillis(), true);
    cisternLevel->setMeasurement(60.0f, Terra_Unit_Percent, terraMillis(), true);
    thermalTankTemperature->setMeasurement(54.0f, Terra_Unit_Celsius, terraMillis(), true);

    terraController.update();
    delay(1000);
}
