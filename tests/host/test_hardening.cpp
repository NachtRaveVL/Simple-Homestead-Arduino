#include "Terraduino.h"
#include <cassert>

static void testCisternReserveAndTransferLimits()
{
    TerraCistern source(1000.0f, 1, "Source");
    assert(source.setThresholds(20.0f, 30.0f, 90.0f));
    source.setStoredLiters(500.0f);

    assert(isFPEqual(source.drawWater(400.0f), 300.0f));
    assert(isFPEqual(source.getStoredLiters(), 200.0f));
    assert(isFPEqual(source.drawWater(50.0f), 0.0f));
    assert(isFPEqual(source.drawWater(50.0f, true), 50.0f));
    assert(isFPEqual(source.getStoredLiters(), 150.0f));

    source.setStoredLiters(500.0f);
    TerraCistern destination(1000.0f, 2, "Destination");
    assert(destination.configureFillBand(35.0f, 90.0f, 99.0f));
    destination.setStoredLiters(850.0f);
    TerraWaterBalancer balancer;
    assert(isFPEqual(balancer.transferAllowance(source, destination, 500.0f), 50.0f));
}

static void testFirstFlushAcrossRainEvents()
{
    TerraCistern cistern(1000.0f, 3, "Rain Cistern");
    assert(cistern.configureFillBand(35.0f, 90.0f, 99.0f));
    TerraRainCatchment catchment(10.0f, 1.0f, 4, "Roof");
    TerraFirstFlushController flush(10.0f);

    TerraRainCollectionResult first = catchment.collectInto(cistern, 0.5f, &flush);
    assert(isFPEqual(first.capturedLiters, 5.0f));
    assert(isFPEqual(first.discardedLiters, 5.0f));
    assert(isFPEqual(first.storedLiters, 0.0f));
    assert(flush.shouldDivert());

    TerraRainCollectionResult second = catchment.collectInto(cistern, 0.8f, &flush);
    assert(isFPEqual(second.capturedLiters, 8.0f));
    assert(isFPEqual(second.discardedLiters, 5.0f));
    assert(isFPEqual(second.storedLiters, 3.0f));
    assert(!flush.shouldDivert());
}

static void testRouteFlowFaultBoundaries()
{
    TerraWaterRoute route(5, "Route");
    route.setMinimumFlow(1.0f);
    route.setMaximumFlow(10.0f);

    assert(!route.validateFlow(12.0f, true));
    assert(route.hasFault());
    assert(route.validateFlow(5.0f, true));
    assert(!route.hasFault());
    assert(!route.validateFlow(2.0f, false));
    assert(route.hasFault());
    assert(route.validateFlow(0.5f, false));
    assert(!route.hasFault());
}

static void testCisternPersistenceRejectsUnsafeBand()
{
    TerraCisternData data;
    data.key = 6;
    data.name = "Unsafe Cistern";
    data.level = 50.0f;
    data.reserveLevel = 10.0f;
    data.lowLevel = 25.0f;
    data.highLevel = 90.0f;
    data.capacityLiters = 1000.0f;
    data.fillStartPercent = 40.0f;
    data.fillStopPercent = 99.0f;
    data.overflowPercent = 99.0f;

    TerraCisternData decoded;
    assert(!decoded.fromJSON(data.toJSON()));
    assert(TerraFactory::newObjectFromData(&data) == nullptr);
}

static TerraThermalStore *restoreThermalStore(float minimumC, float maximumC, float absoluteMaximumC, uint32_t key)
{
    TerraThermalStoreData data;
    data.key = key;
    data.name = "Thermal Store";
    data.minimumTargetC = minimumC;
    data.maximumTargetC = maximumC;
    data.absoluteMaximumC = absoluteMaximumC;

    TerraThermalStoreData decoded;
    assert(decoded.fromJSON(data.toJSON()));
    TerraObject *object = TerraFactory::newObjectFromData(&decoded);
    assert(object != nullptr);
    assert(object->getObjectType() == Terra_ObjectType_ThermalStore);
    return static_cast<TerraThermalStore *>(object);
}

static void testFactoryRejectsInvalidRuntimeConfiguration()
{
    TerraResourceData resource;
    resource.key = 9;
    resource.resourceType = Terra_ResourceType_Water;
    resource.reserveLevel = 50.0f;
    resource.lowLevel = 20.0f;
    resource.highLevel = 90.0f;
    assert(TerraFactory::newObjectFromData(&resource) == nullptr);

    TerraThermalLoop loop(10, "Loop");
    assert(!loop.configure(3.0f, 3.0f, 80.0f));

    TerraThermalLoopData loopData;
    loopData.key = 10;
    loopData.onDifferentialC = 3.0f;
    loopData.offDifferentialC = 3.0f;
    loopData.maxStoreTempC = 80.0f;
    assert(TerraFactory::newObjectFromData(&loopData) == nullptr);
}

static void testThermalFactoryRestoresSafetyRange()
{
    TerraThermalStore *high = restoreThermalStore(100.0f, 120.0f, 130.0f, 7);
    assert(isFPEqual(high->getMinimumTarget(), 100.0f));
    assert(isFPEqual(high->getMaximumTarget(), 120.0f));
    assert(isFPEqual(high->getAbsoluteMaximum(), 130.0f));
    delete high;

    TerraThermalStore *low = restoreThermalStore(20.0f, 60.0f, 70.0f, 8);
    assert(isFPEqual(low->getMinimumTarget(), 20.0f));
    assert(isFPEqual(low->getMaximumTarget(), 60.0f));
    assert(isFPEqual(low->getAbsoluteMaximum(), 70.0f));
    delete low;
}

int main()
{
    testCisternReserveAndTransferLimits();
    testFirstFlushAcrossRainEvents();
    testRouteFlowFaultBoundaries();
    testCisternPersistenceRejectsUnsafeBand();
    testFactoryRejectsInvalidRuntimeConfiguration();
    testThermalFactoryRestoresSafetyRange();
    return 0;
}
