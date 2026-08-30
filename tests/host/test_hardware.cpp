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

    TerraActivationHandle first = pump->enableActuator(Terra_DirectionMode_Forward, 0.4f);
    TerraActivationHandle second = pump->enableActuator(Terra_DirectionMode_Forward, 0.8f);
    pump->setEnableMode(Terra_EnableMode_Highest);
    pump->update(100);
    assert(pump->isEnabled());
    assert(isFPEqual(pump->getDriveIntensity(), 1.0f));

    first.unset();
    second.unset();
    pump->update(101);
    assert(!pump->isEnabled());

    auto valve = controller.addValveRelay(7, true, "Valve");
    assert(valve);
    assert(valve->getActuatorType() == Terra_ActuatorType_Valve);

    auto leak = controller.addLeakIndicator(9, true, "Leak");
    assert(leak);
    leak->setSimulatedState(true);
    assert(leak->takeMeasurement(true));
    assert(leak->isActive(false));

    std::cout << "PASS Terraduino hardware" << std::endl;
    return 0;
}
