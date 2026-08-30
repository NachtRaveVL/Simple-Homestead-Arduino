// Simple-Homestead-Arduino Cistern Management Example
//
// Demonstrates reservoir level sensing and relay pump control for a primary cistern. The
// MCU output must drive a suitable isolated relay, contactor, or motor-control interface
// sized for the actual pump.

#include <Terraduino.h>

#define SETUP_FILL_PUMP_PIN             5                    // Isolated fill-pump relay control pin
#define SETUP_FILL_PUMP_ACTIVE_LOW      true                 // Relay control is active-low
Terraduino terraController;
SharedPtr<TerraInfiniteWaterReservoir> well;
SharedPtr<TerraWaterReservoir> cistern;
SharedPtr<TerraRelayPumpActuator> fillPump;
SharedPtr<TerraRemoteSensor> cisternVolume;
bool filling = false;

void setup()
{
    Serial.begin(115200);

    terraController.init();
    well = terraController.addInfiniteWaterReservoir(true, "Well");
    cistern = terraController.addWaterReservoir(5000.0f, "Main Cistern");
    fillPump = terraController.addPumpRelay(SETUP_FILL_PUMP_PIN, SETUP_FILL_PUMP_ACTIVE_LOW, "Fill Pump");
    cisternVolume = terraController.addRemoteSensor(Terra_SensorType_Level,
                                                   Terra_UnitsType_LiqVolume_Liters,
                                                   "Cistern Volume");

    cistern->getWaterVolumeSensorAttachment().setObject(cisternVolume);
    fillPump->getSourceReservoirAttachment().setObject(well);
    fillPump->getDestinationReservoirAttachment().setObject(cistern);

    terraController.launch();
}

void loop()
{
    // Replace with the installed volume sensor driver.
    cisternVolume->receiveReport(2500.0f, Terra_UnitsType_LiqVolume_Liters);

    terraController.update();

    float level = cistern->getLevel(true);
    if (level <= 30.0f) { filling = true; }
    if (level >= 90.0f) { filling = false; }

    if (filling && fillPump->canPump((millis_t)1000)) {
        fillPump->pump((millis_t)1000);
    }

    delay(1000);
}
