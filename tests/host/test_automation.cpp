#include "Terraduino.h"
#include <cassert>
#include <iostream>

int main()
{
    Terraduino controller;
    controller.init();

    auto source = controller.addInfiniteWaterReservoir(true);
    auto destination = controller.addWaterReservoir(1000.0f);
    auto pump = controller.addPumpRelay(5, false);
    assert(source && destination && pump);

    pump->getSourceReservoirAttachment().setObject(source);
    pump->getDestinationReservoirAttachment().setObject(destination);
    assert(pump->getSourceReservoirAttachment().getKey() == source->getKey());
    assert(pump->getDestinationReservoirAttachment().getKey() == destination->getKey());

    auto temperature = controller.addRemoteSensor(Terra_SensorType_Temperature, Terra_UnitsType_Temperature_Celsius);
    assert(temperature);

    TerraLinearEdgeBalancer balancer(temperature, 50.0f, 4.0f);
    balancer.setEnabled(true);
    controller.launch();

    temperature->receiveReport(40.0f, Terra_UnitsType_Temperature_Celsius, 100, true);
    assert(balancer.getBalancingState(false) == Terra_BalancingState_TooLow);

    temperature->receiveReport(50.0f, Terra_UnitsType_Temperature_Celsius, 101, true);
    assert(balancer.getBalancingState(false) == Terra_BalancingState_Balanced);

    temperature->receiveReport(60.0f, Terra_UnitsType_Temperature_Celsius, 102, true);
    assert(balancer.getBalancingState(false) == Terra_BalancingState_TooHigh);

    std::cout << "PASS Terraduino automation" << std::endl;
    return 0;
}
