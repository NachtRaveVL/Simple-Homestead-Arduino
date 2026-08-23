#include "Terraduino.h"
#include <cassert>
#include <iostream>

int main()
{
    Terraduino controller;
    controller.init();

    auto environment = controller.addEnvironment("Outside");
    auto pressure = controller.addRemoteSensor(Terra_SensorType_Pressure, Terra_UnitsType_Hectopascals, "Pressure");
    assert(environment && pressure);
    pressure->receiveReport(1013.25f, Terra_UnitsType_Hectopascals, 100, true);
    environment->setPressureSensor(pressure);
    environment->update(100);
    assert(isFPEqual(environment->getBarometricPressure(), 1013.25f));

    auto cistern = controller.addCistern(5000.0f, "Main Cistern");
    auto level = controller.addAnalogLevelSensor(6, 0.1f, 0.9f, 0.0f, 100.0f, "Cistern Level");
    assert(cistern && level);
    cistern->setLevelSensor(level);

    TerraObjectData *data = cistern->newSaveData();
    assert(data);
    auto *cisternData = static_cast<TerraCisternData *>(data);
    assert(cisternData->levelSensor[0]);
    TerraObject *restored = TerraFactory::newObjectFromData(data);
    assert(restored);
    auto *restoredCistern = static_cast<TerraCistern *>(restored);
    assert(restoredCistern->getLevelSensorAttachment().getKey() == level->getKey());
    delete restored;
    delete data;

    std::cout << "PASS Terraduino" << std::endl;
    return 0;
}
