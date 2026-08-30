#include "Terraduino.h"
#include <cassert>
#include <iostream>

int main()
{
    assert(terraBalancingStateForValue(47.0f, 50.0f, 4.0f) == Terra_BalancingState_TooLow);
    assert(terraBalancingStateForValue(50.0f, 50.0f, 4.0f) == Terra_BalancingState_Balanced);
    assert(terraBalancingStateForValue(53.0f, 50.0f, 4.0f) == Terra_BalancingState_TooHigh);
    assert(terraBalancingCorrectionForState(Terra_BalancingState_TooLow) == 1);
    assert(terraBalancingCorrectionForState(Terra_BalancingState_Balanced) == 0);
    assert(terraBalancingCorrectionForState(Terra_BalancingState_TooHigh) == -1);

    Terraduino controller;
    controller.init();
    auto reservoir = controller.addWaterReservoir(1000.0f, "Reservoir");
    assert(reservoir);
    reservoir->setFault("test");
    assert(reservoir->getState(false) == Terra_ResourceState_Unknown);
    reservoir->clearFault();

    auto catchment = controller.addRainCatchment(100.0f, 0.8f, "Roof");
    assert(catchment);
    assert(isFPEqual(catchment->estimateCaptureLiters(10.0f), 800.0f));

    TerraFirstFlushController firstFlush(20.0f);
    assert(isFPEqual(firstFlush.getRemainingLiters(), 20.0f));
    firstFlush.recordFlow(7.5f);
    assert(isFPEqual(firstFlush.getRemainingLiters(), 12.5f));

    std::cout << "PASS Terraduino hardening" << std::endl;
    return 0;
}
