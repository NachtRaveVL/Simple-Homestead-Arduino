// Simple-Homestead-Arduino Cistern Management Example
//
// Demonstrates fill hysteresis, protected reserve, level sensing, and relay pump control
// for a primary cistern. The MCU output must drive a suitable isolated relay, contactor,
// or motor-control interface sized for the actual pump.

#include <Terraduino.h>

#define SETUP_FILL_PUMP_PIN             5                    // Isolated fill-pump relay control pin
#define SETUP_FILL_PUMP_ACTIVE_LOW      true                 // Relay control is active-low
#define SETUP_CISTERN_LEVEL_PIN         A0                   // Analog cistern-level sensor input
#define SETUP_CISTERN_LEVEL_EMPTY       120.0f               // Raw reading at empty level
#define SETUP_CISTERN_LEVEL_FULL        900.0f               // Raw reading at full level

Terraduino terraController;
SharedPtr<TerraWaterSource> well;
SharedPtr<TerraCistern> cistern;
SharedPtr<TerraWaterRoute> fillRoute;
SharedPtr<TerraPump> fillPump;
SharedPtr<TerraLevelSensor> cisternLevel;

void setup()
{
    Serial.begin(115200);

    terraController.init();
    well = terraController.addWaterSource(Terra_WaterSourceType_Well, "Well");
    cistern = terraController.addCistern(5000.0f, "Main Cistern");
    fillRoute = terraController.addWaterRoute("Cistern Fill");
    fillPump = terraController.addPumpRelay(SETUP_FILL_PUMP_PIN, SETUP_FILL_PUMP_ACTIVE_LOW, "Fill Pump");
    cisternLevel = terraController.addAnalogLevelSensor(SETUP_CISTERN_LEVEL_PIN, "Cistern Level");

    TerraCalibrationData cisternLevelCalibration(cisternLevel->getKey(), Terra_Unit_Percent);
    cisternLevelCalibration.setFromTwoPoints(SETUP_CISTERN_LEVEL_EMPTY, 0.0f,
                                              SETUP_CISTERN_LEVEL_FULL, 100.0f);
    cisternLevel->setUserCalibrationData(&cisternLevelCalibration);

    well->setLevel(100.0f);
    well->setReserveLevel(10.0f);
    well->setMaximumFlowLpm(20.0f);

    cistern->setThresholds(15.0f, 30.0f, 95.0f);
    cistern->configureFillBand(30.0f, 90.0f, 99.0f);
    cistern->setLevelSensor(cisternLevel);

    fillPump->setMaxContinuousRuntime(15UL * 60UL * 1000UL);

    fillRoute->setSource(well);
    fillRoute->setDestination(cistern);
    fillRoute->setPump(fillPump);
    fillRoute->setDestinationBand(30.0f, 90.0f);
    fillRoute->setMaximumFlow(20.0f);

    terraController.launch();
}

void loop()
{
    // The route owns fill hysteresis and pump control during controller updates.
    terraController.update();
    delay(1000);
}
