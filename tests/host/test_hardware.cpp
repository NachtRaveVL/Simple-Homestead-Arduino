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
    TerraCalibrationData levelCalibration(level.getKey(), Terra_Unit_Percent);
    levelCalibration.setFromTwoPoints(0.0f, 0.0f, 1000.0f, 100.0f);
    level.setUserCalibrationData(&levelCalibration);
    check(level.getUserCalibrationData() == &levelCalibration,
          "standalone sensor accepts user calibration data");
    level.setDriver(SharedPtr<TerraInputDriver>(new TerraCallbackInputDriver(readRaw, nullptr, Terra_Unit_Raw)));

    rawValue = 250.0f;
    level.update(1000);
    check(isFPEqual(level.getMeasurement().value, 25.0f), "sensor maps raw reading into engineering units");
    check(level.getMeasurement().unit == Terra_Unit_Percent, "sensor calibration updates engineering units");
    level.update(1500);
    check(isFPEqual(level.getMeasurement().value, 25.0f), "sensor does not recalibrate unchanged measurement");
    rawValue = 500.0f;
    level.update(2000);
    check(isFPEqual(level.getMeasurement().value, 50.0f), "sensor calibrates only a new driver reading");

    TerraPump arbitrationPump(2, "Arbitration Pump");
    TerraActivationHandle firstRequest(&arbitrationPump, 0.4f, (millis_t)-1);
    TerraActivationHandle secondRequest(&arbitrationPump, 0.8f, (millis_t)-1);
    firstRequest.enable();
    secondRequest.enable();

    arbitrationPump.setEnableMode(Terra_EnableMode_Highest);
    arbitrationPump.update(100);
    check(isFPEqual(arbitrationPump.getOutput(), 0.8f), "highest activation mode selects highest request");
    arbitrationPump.setEnableMode(Terra_EnableMode_Lowest);
    arbitrationPump.update(101);
    check(isFPEqual(arbitrationPump.getOutput(), 0.4f), "lowest activation mode selects lowest request");
    arbitrationPump.setEnableMode(Terra_EnableMode_InOrder);
    arbitrationPump.update(102);
    check(isFPEqual(arbitrationPump.getOutput(), 0.4f), "in-order activation mode selects first request");
    check(firstRequest.isActive() && !secondRequest.isActive(), "in-order mode only runs selected handle time");
    firstRequest.unset();
    arbitrationPump.update(103);
    check(isFPEqual(arbitrationPump.getOutput(), 0.8f), "in-order activation advances to next request");
    check(secondRequest.isActive(), "next serial activation starts when selected");

    TerraPump timedPump(3, "Timed Pump");
    timedPump.setEnableMode(Terra_EnableMode_InOrder);
    TerraActivationHandle timedFirst(&timedPump, 0.5f, 100);
    TerraActivationHandle timedSecond(&timedPump, 0.7f, 100);
    timedFirst.enable();
    timedSecond.enable();
    timedPump.update(200);
    check(timedFirst.isActive() && !timedSecond.isActive(),
          "serial activation only starts the selected handle");
    millis_t waitingDuration = timedSecond.getTimeLeft();
    timedPump.update(210);
    check(timedSecond.getTimeLeft() == waitingDuration,
          "waiting serial activation does not consume duration");

    Terraduino controller;
    controller.init();

    auto pump = controller.addPumpRelay(5, false, "Pump");
    check(pump != nullptr && pump->getDriver() != nullptr, "factory creates pin-backed pump relay");
    auto pumpDriver = static_pointer_cast<TerraDigitalOutputDriver>(pump->getDriver());
    pump->setOutput(1.0f);
    check(pumpDriver->getLastState(), "pump relay factory drives configured pin");
    pump->off();
    check(!pumpDriver->getLastState(), "pump relay factory releases configured pin");

    auto levelSensor = controller.addAnalogLevelSensor(6, "Cistern Level");
    check(levelSensor != nullptr && levelSensor->getDriver() != nullptr, "factory creates raw analog level sensor");
    check(levelSensor->getSensorType() == Terra_SensorType_Level, "factory preserves level sensor domain type");
    TerraCalibrationData controllerLevelCalibration(levelSensor->getKey(), Terra_Unit_Percent);
    controllerLevelCalibration.setFromTwoPoints(100.0f, 0.0f, 900.0f, 100.0f);
    levelSensor->setUserCalibrationData(&controllerLevelCalibration);
    check(levelSensor->getUserCalibrationData() != nullptr &&
          levelSensor->getUserCalibrationData() != &controllerLevelCalibration,
          "controller owns copied calibration data like Hydro/Helio");
    check(controller.getUserCalibrationData(levelSensor->getKey()) == levelSensor->getUserCalibrationData(),
          "sensor calibration resolves through controller calibration store");

    auto temperature = controller.addAnalogTemperatureSensor(7, "Outdoor Temperature");
    check(temperature != nullptr && temperature->getDriver() != nullptr, "factory creates raw analog temperature sensor");
    check(temperature->getSensorType() == Terra_SensorType_Temperature, "temperature sensor keeps domain type");
    TerraCalibrationData temperatureCalibration(temperature->getKey(), Terra_Unit_Celsius);
    temperatureCalibration.setFromTwoPoints(100.0f, -20.0f, 900.0f, 80.0f);
    temperature->setUserCalibrationData(&temperatureCalibration);
    check(temperature->getUserCalibrationData() != nullptr, "temperature sensor accepts controller-owned calibration data");

    auto sump = controller.addSumpPumpRelay(8, true, "Sump Pump");
    check(sump != nullptr && sump->getDriver() != nullptr, "factory creates active-low sump relay");
    sump->setLevelSensor(levelSensor);
    check(sump->getLevelSensorAttachment().isSet(), "sump uses normal attachment for level sensor");

    auto leak = controller.addLeakIndicator(9, true, "Leak Sensor");
    check(leak != nullptr && leak->getDriver() != nullptr, "factory creates pin-backed leak indicator");

    std::cout << "PASS Terraduino hardware" << std::endl;
    return 0;
}
