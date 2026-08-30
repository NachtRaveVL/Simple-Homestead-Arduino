// Simple-Homestead-Arduino Local Dashboard Example
//
// Keeps a small local status display on the serial console without requiring networking
// or the project-specific tcMenu screens, which are still under development.

#include <Terraduino.h>

Terraduino terraController;
SharedPtr<TerraWaterReservoir> waterTank;
SharedPtr<TerraRemoteSensor> waterVolume;
SharedPtr<TerraRemoteSensor> outsideTemperature;
uint32_t lastReport = 0;

void setup()
{
    Serial.begin(115200);

    terraController.init();
    waterTank = terraController.addWaterReservoir(5000.0f, "Main Water Tank");
    waterVolume = terraController.addRemoteSensor(Terra_SensorType_Level,
                                                  Terra_UnitsType_LiqVolume_Liters,
                                                  "Water Volume");
    outside = terraController.addEnvironment("Outside");
    outsideTemperature = terraController.addRemoteSensor(Terra_SensorType_Temperature,
                                                         Terra_UnitsType_Temperature_Celsius,
                                                         "Outside Temperature");

    waterTank->getWaterVolumeSensorAttachment().setObject(waterVolume);
    outside->setAirTemperatureSensor(outsideTemperature);
    terraController.launch();
}

void loop()
{
    // Replace these sample reports with installed sensor hardware.
    waterVolume->receiveReport(3200.0f, Terra_UnitsType_LiqVolume_Liters);
    outsideTemperature->receiveReport(4.0f, Terra_UnitsType_Temperature_Celsius);

    terraController.update();

    uint32_t now = millis();
    if (!lastReport || now - lastReport >= 5000UL) {
        Serial.print(F("Water tank: "));
        Serial.print(waterTank->getLevel(), 1);
        Serial.print(F("%  Outside: "));
        Serial.print(outside->getAirTemperature(), 1);
        Serial.println(F(" C"));
        lastReport = now;
    }
}
