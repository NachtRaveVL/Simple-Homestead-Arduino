#include "Terraduino.h"
#include <cassert>
#include <iostream>

int main()
{
    Terraduino controller;
    controller.init();

    auto pump = controller.addPumpRelay(5, false, "Pump");
    assert(pump);
    assert(pump->getOutputPin().isValid());

    TerraActivationHandle first = pump->activate(Terra_DirectionMode_Forward, 0.4f);
    TerraActivationHandle second = pump->activate(Terra_DirectionMode_Forward, 0.8f);
    pump->setEnableMode(Terra_EnableMode_Highest);
    pump->update(100);
    assert(pump->isEnabled());
    assert(isFPEqual(pump->getDriveIntensity(), 1.0f));

    first.unset();
    second.unset();
    pump->update(101);
    assert(!pump->isEnabled());

    auto sump = controller.addSumpPumpRelay(7, true, "Sump");
    auto level = controller.addAnalogLevelSensor(8, 0.0f, 1.0f, 0.0f, 100.0f, "Sump Level");
    assert(sump && level);
    sump->setLevelSensor(level);
    assert(sump->getLevelSensorAttachment().isSet());

    auto leak = controller.addLeakIndicator(9, true, "Leak");
    assert(leak);
    leak->setSimulatedState(true);
    assert(leak->takeMeasurement(true));
    assert(leak->isActive(false));

    std::cout << "PASS Terraduino hardware" << std::endl;
    return 0;
}
