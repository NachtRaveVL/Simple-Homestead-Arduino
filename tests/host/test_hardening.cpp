#include "Terraduino.h"
#include <cassert>
#include <iostream>

int main()
{
    TerraCistern cistern(1000.0f, 0, "Cistern");
    assert(cistern.setThresholds(10.0f, 25.0f, 90.0f));
    assert(!cistern.setThresholds(30.0f, 20.0f, 90.0f));
    assert(cistern.configureFillBand(30.0f, 90.0f, 99.0f));
    assert(!cistern.configureFillBand(40.0f, 99.0f, 99.0f));

    TerraThermalStore store(0, "Store");
    assert(store.setTargetRange(40.0f, 60.0f));
    assert(!store.setTargetRange(70.0f, 60.0f));
    assert(store.setAbsoluteMaximum(90.0f));
    assert(!store.setTargetRange(40.0f, 95.0f));

    TerraWaterBalancer balancer;
    TerraCistern source(1000.0f, 0, "Source");
    TerraCistern destination(1000.0f, 1, "Destination");
    source.setThresholds(20.0f, 30.0f, 90.0f);
    source.setStoredLiters(500.0f);
    destination.configureFillBand(35.0f, 90.0f, 99.0f);
    destination.setStoredLiters(850.0f);
    assert(isFPEqual(balancer.transferAllowance(source, destination, 500.0f), 50.0f));

    std::cout << "PASS Terraduino hardening" << std::endl;
    return 0;
}
