#include "Terraduino.h"
#include <cassert>
#include <iostream>

int main()
{
    Terraduino controller;
    controller.init();

    auto pump = controller.addPumpRelay(5, false);
    assert(pump);
    assert(pump->getOutputPin().isValid());

    TerraCalibrationData calibration(pump->getId(), Terra_UnitsType_Raw_1);
    calibration.setFromTwoPoints(0.0f, 0.0f, 1.0f, 1.0f);
    pump->setUserCalibrationData(&calibration);
    assert(pump->getUserCalibrationData() != nullptr);
    controller.clearUserCalibrations();
    assert(pump->getUserCalibrationData() == nullptr);

    TerraActivationHandle first = pump->enableActuator(Terra_DirectionMode_Forward, 0.4f);
    TerraActivationHandle second = pump->enableActuator(Terra_DirectionMode_Forward, 0.8f);
    pump->setEnableMode(Terra_EnableMode_Highest);
    pump->update();
    assert(pump->isEnabled());
    assert(isFPEqual(pump->getDriveIntensity(), 1.0f));

    first.unset();
    second.unset();
    pump->update();
    assert(!pump->isEnabled());

    auto valve = controller.addValveRelay(7, true);
    assert(valve);
    assert(valve->getActuatorType() == Terra_ActuatorType_Valve);

    auto leak = controller.addLeakIndicator(9, true);
    assert(leak);
    digitalWrite(9, LOW);
    assert(leak->takeMeasurement(true));
    assert(leak->isActive(false));

    std::cout << "PASS Terraduino hardware" << std::endl;
    return 0;
}
