#include "Terraduino.h"
#include <cassert>
#include <iostream>

int main()
{
    Terraduino controller;
    controller.init();

    auto first = controller.addWaterReservoir(1000.0f);
    auto second = controller.addWaterReservoir(1000.0f);
    assert(first && second);
    assert(first->getId().isReservoirType());
    assert(first->getId().objTypeAs.reservoirType == Terra_ReservoirType_Water);
    assert(first->getId().posIndex != second->getId().posIndex);
    assert(first->getKey() != second->getKey());

    TerraSingleMeasurement celsius(20.0f, Terra_UnitsType_Temperature_Celsius, 100, 1);
    TerraSingleMeasurement fahrenheit = celsius.asUnits(Terra_UnitsType_Temperature_Fahrenheit);
    assert(fahrenheit.isSet());
    assert(isFPEqual(fahrenheit.value, 68.0f));

    auto level = controller.addRemoteSensor(Terra_SensorType_Level, Terra_UnitsType_LiqVolume_Liters);
    assert(level);
    level->receiveReport(500.0f, Terra_UnitsType_LiqVolume_Liters, 200, true);
    const TerraMeasurement *measurement = level->getMeasurement(false);
    assert(measurement && measurement->isSingleType());
    assert(getMeasurementUnits(measurement) == Terra_UnitsType_LiqVolume_Liters);

    TerraData *saved = first->newSaveData();
    assert(saved && saved->isObjectData());
    TerraObject *restored = newObjectFromData(static_cast<TerraObjectData *>(saved));
    assert(restored);
    assert(restored->getId() == first->getId());
    delete restored;
    delete saved;

    std::cout << "PASS Terraduino infrastructure" << std::endl;
    return 0;
}
