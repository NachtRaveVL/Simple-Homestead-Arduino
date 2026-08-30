// Simple-Homestead-Arduino Data Writer Example
//
// Builds a small controller configuration and exports it through the same TerraData /
// ArduinoJson persistence path used by normal controller saves.

#include <Terraduino.h>

Terraduino terraController;

void setup()
{
    Serial.begin(115200);
    while (!Serial) { ; }

    terraController.init();

    auto reservoir = terraController.addWaterReservoir(5000.0f, "Main Reservoir");
    auto volume = terraController.addRemoteSensor(Terra_SensorType_Level,
                                                   Terra_UnitsType_LiqVolume_Liters,
                                                   "Reservoir Volume");
    reservoir->getWaterVolumeSensorAttachment().setObject(volume);
    volume->receiveReport(3175.0f, Terra_UnitsType_LiqVolume_Liters);

    auto thermal = terraController.addThermalReservoir(90.0f, "Thermal Store");
    auto temperature = terraController.addRemoteSensor(Terra_SensorType_Temperature,
                                                        Terra_UnitsType_Temperature_Celsius,
                                                        "Thermal Store Temperature");
    thermal->getMediumTemperatureSensorAttachment().setObject(temperature);
    temperature->receiveReport(58.0f, Terra_UnitsType_Temperature_Celsius);

    Serial.println(terraController.exportSystemJSON().c_str());
}

void loop() { ; }
