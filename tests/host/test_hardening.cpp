#include "TerraCoreLogic.h"
#include <cassert>
#include <cstdint>
#include <iostream>

int main()
{
    assert(terraBalancingStateForValue(47.0f, 50.0f, 4.0f) == 0);
    assert(terraBalancingStateForValue(50.0f, 50.0f, 4.0f) == 1);
    assert(terraBalancingStateForValue(53.0f, 50.0f, 4.0f) == 2);
    assert(terraBalancingCorrectionForState(0) == 1);
    assert(terraBalancingCorrectionForState(1) == 0);
    assert(terraBalancingCorrectionForState(2) == -1);

    bool pendingState = true;
    bool hasPendingState = true;
    uint32_t pendingStart = UINT32_MAX - 50;
    bool accepted = false;

    accepted = terraUpdateStableBinaryState(accepted, true, 48, 100,
                                             pendingState, hasPendingState, pendingStart);
    assert(!accepted && hasPendingState);

    accepted = terraUpdateStableBinaryState(accepted, true, 49, 100,
                                             pendingState, hasPendingState, pendingStart);
    assert(accepted && !hasPendingState);

    auto baseOnly = terraBinaryDataReadPlan(20, 100, 20);
    assert(baseOnly.copyBytes == 0);
    assert(baseOnly.skipBytes == 0);

    std::cout << "PASS Terraduino hardening" << std::endl;
    return 0;
}
