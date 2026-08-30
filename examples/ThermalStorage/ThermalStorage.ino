// Simple-Homestead-Arduino Thermal Storage Example
//
// Demonstrates the canonical thermal reservoir with a temperature sensor attachment.
// Independent hardware temperature and pressure protection remains responsible for safety.

#include <Terraduino.h>

Terraduino terraController;
SharedPtr<TerraRemoteSensor> storeTemperature;
SharedPtr<TerraThermalReservoir> heatStore;

void setup()
{
    Serial.begin(115200);

    terraController.init();
    storeTemperature = terraController.addRemoteSensor(Terra_SensorType_Temperature,
                                                       Terra_UnitsType_Temperature_Celsius,
                                                       "Store Temperature");
    heatStore = terraController.addThermalReservoir(90.0f, "Hot Water Store");
    heatStore->getMediumTemperatureSensorAttachment().setObject(storeTemperature);

    terraController.launch();
}

void loop()
{
    // Replace with the installed temperature sensor driver.
    storeTemperature->receiveReport(52.0f, Terra_UnitsType_Temperature_Celsius);

    terraController.update();

    Serial.print(F("Thermal store level, %: "));
    Serial.println(heatStore->getLevel(true), 1);
    delay(1000);
}
