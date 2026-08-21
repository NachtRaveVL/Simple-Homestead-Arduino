#include "Terraduino.h"
#include <cstdlib>
#include <iostream>

static void check(bool condition, const char *message)
{
    if (!condition) {
        std::cerr << "FAIL: " << message << std::endl;
        std::exit(1);
    }
}

static float rawValue = 0.0f;
static float readRaw(void *)
{
    return rawValue;
}

int main()
{
    TerraLevelSensor level(1, "Level");
    level.setUpdateInterval(1000);
    check(level.setCalibration(0.0f, 1000.0f, 0.0f, 100.0f, Terra_Unit_Percent),
          "sensor accepts linear calibration");
    level.setDriver(SharedPtr<TerraInputDriver>(new TerraCallbackInputDriver(readRaw, nullptr, Terra_Unit_Raw)));

    rawValue = 250.0f;
    level.update(1000);
    check(isFPEqual(level.getMeasurement().value, 25.0f), "sensor maps raw reading into engineering units");
    level.update(1500);
    check(isFPEqual(level.getMeasurement().value, 25.0f), "sensor does not recalibrate unchanged measurement");
    rawValue = 500.0f;
    level.update(2000);
    check(isFPEqual(level.getMeasurement().value, 50.0f), "sensor recalibrates only a new driver reading");

    Terraduino controller;
    controller.init();

    auto pump = controller.addPumpRelay(5, false, "Pump");
    check(pump != nullptr && pump->getDriver() != nullptr, "factory creates pin-backed pump relay");
    auto pumpDriver = static_pointer_cast<TerraDigitalOutputDriver>(pump->getDriver());
    pump->setOutput(1.0f);
    check(pumpDriver->getLastState(), "pump relay factory drives configured pin");
    pump->off();
    check(!pumpDriver->getLastState(), "pump relay factory releases configured pin");

    auto levelSensor = controller.addAnalogLevelSensor(6, 100.0f, 900.0f, 0.0f, 100.0f, "Cistern Level");
    check(levelSensor != nullptr && levelSensor->getDriver() != nullptr, "factory creates calibrated analog level sensor");
    check(levelSensor->hasCalibration(), "factory stores analog calibration on sensor object");
    check(levelSensor->getSensorType() == Terra_SensorType_Level, "factory preserves level sensor domain type");

    auto temperature = controller.addAnalogTemperatureSensor(7, 100.0f, 900.0f, -20.0f, 80.0f,
                                                              Terra_Unit_Celsius, "Outdoor Temperature");
    check(temperature != nullptr && temperature->hasCalibration(), "factory creates calibrated temperature sensor");
    check(temperature->getSensorType() == Terra_SensorType_Temperature, "temperature sensor keeps domain type");

    auto sump = controller.addSumpPumpRelay(8, true, "Sump Pump");
    check(sump != nullptr && sump->getDriver() != nullptr, "factory creates active-low sump relay");
    sump->setLevelSensor(levelSensor);
    check(sump->getLevelSensorAttachment().isSet(), "sump uses normal attachment for level sensor");

    auto leak = controller.addLeakIndicator(9, true, "Leak Sensor");
    check(leak != nullptr && leak->getDriver() != nullptr, "factory creates pin-backed leak indicator");

    std::cout << "PASS Terraduino hardware" << std::endl;
    return 0;
}
