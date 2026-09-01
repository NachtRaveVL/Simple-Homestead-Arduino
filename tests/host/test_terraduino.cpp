#include "Terraduino.h"
#include <cassert>
#include <iostream>

int main()
{
    Terraduino controller;
    controller.init();

    auto pressure = controller.addRemoteSensor(Terra_SensorType_Pressure, Terra_UnitsType_Pressure_Hectopascals);
    assert(pressure);
    pressure->receiveReport(1013.25f, Terra_UnitsType_Pressure_Hectopascals, 100, true);
    assert(isFPEqual(getMeasurementValue(pressure->getMeasurement(false)), 1013.25f));

    auto reservoir = controller.addWaterReservoir(5000.0f);
    auto volume = controller.addRemoteSensor(Terra_SensorType_Level, Terra_UnitsType_LiqVolume_Liters);
    assert(reservoir && volume);
    reservoir->getWaterVolumeSensorAttachment().setObject(volume);
    volume->receiveReport(2500.0f, Terra_UnitsType_LiqVolume_Liters, 100, true);
    assert(isFPEqual(reservoir->getLevel(true), 50.0f));

    TerraData *data = reservoir->newSaveData();
    assert(data);
    auto *reservoirData = static_cast<TerraWaterReservoirData *>(data);
    assert(reservoirData->volumeSensor[0]);

    TerraObject *restored = TerraFactory::newObjectFromData(reservoirData);
    assert(restored);
    auto *restoredReservoir = static_cast<TerraWaterReservoir *>(restored);
    assert(restoredReservoir->getWaterVolumeSensorAttachment().getKey() == volume->getKey());
    delete restored;
    delete data;

    std::cout << "PASS Terraduino" << std::endl;
    return 0;
}
