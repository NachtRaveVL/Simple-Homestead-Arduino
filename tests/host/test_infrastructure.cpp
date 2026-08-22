#include "Terraduino.h"
#include <cassert>
#include <cmath>
#include <iostream>

static int logEvents = 0;
static void handleLogEvent(const TerraLogEvent event)
{
    ++logEvents;
    assert(event.level == Terra_LogLevel_Info);
}

static int publishEvents = 0;
static void handlePublishEvent(Pair<uint8_t, const TerraDataColumn *> event)
{
    ++publishEvents;
    assert(event.first == 1);
    assert(event.second != nullptr);
    assert(event.second[0].measurement.isSet());
}

int main()
{
    Terraduino controller;
    controller.init();

    auto first = controller.addCistern(1000.0f, "First");
    auto second = controller.addCistern(1000.0f, "Second");
    assert(first && second);
    assert(first->getId().type == Terra_ObjectType_WaterStorage);
    assert(first->getId().objTypeAs.waterStorageType == Terra_WaterStorageType_Cistern);
    assert(first->getId().posIndex != second->getId().posIndex);
    assert(first->getKey() != second->getKey());

    TerraSingleMeasurement celsius(20.0f, Terra_Unit_Celsius, 100, 1);
    TerraSingleMeasurement fahrenheit = celsius.asUnits(Terra_Unit_Fahrenheit);
    assert(fahrenheit.isSet());
    assert(isFPEqual(fahrenheit.value, 68.0f));

    auto level = controller.addAnalogLevelSensor(6, 0.1f, 0.9f, 0.0f, 100.0f, "Level");
    assert(level);
    level->setSimulatedValue(512);
    assert(level->takeMeasurement(true));
    const TerraMeasurement *measurement = level->getMeasurement(false);
    assert(measurement && measurement->isSingleType());
    assert(getMeasurementUnits(measurement) == Terra_Unit_Percent);

    FunctionSlot<const TerraLogEvent> logSlot(handleLogEvent);
    controller.getLogger().getLogSignal().attach(logSlot);
    controller.getLogger().info("test", "signal");
    assert(logEvents == 1);

    FunctionSlot<Pair<uint8_t, const TerraDataColumn *>> publishSlot(handlePublishEvent);
    controller.getPublisher().getPublishSignal().attach(publishSlot);
    assert(controller.getPublisher().addColumn(level->getKey()));
    level->setMeasurement(50.0f, Terra_Unit_Percent, 200, true);
    assert(publishEvents == 1);

    TerraObjectData *saved = first->newSaveData();
    assert(saved && saved->isObjectData());
    TerraObject *restored = TerraFactory::newObjectFromData(saved);
    assert(restored);
    assert(restored->getId() == first->getId());
    delete restored;
    delete saved;

    std::cout << "PASS Terraduino infrastructure" << std::endl;
    return 0;
}
