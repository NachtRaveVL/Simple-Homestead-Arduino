#include "Terraduino.h"
#include <cassert>
#include <iostream>

int main()
{
    Terraduino controller;
    controller.init();

    auto source = controller.addWaterSource(Terra_WaterSourceType_Well, 0, "Well");
    auto cistern = controller.addCistern(1000.0f, "Cistern");
    auto pump = controller.addPumpRelay(5, false, "Fill Pump");
    auto route = controller.addWaterRoute("Fill Route");
    assert(source && cistern && pump && route);

    source->setLevel(100.0f);
    source->setReserveLevel(10.0f);
    cistern->setLevel(20.0f);
    cistern->configureFillBand(30.0f, 90.0f, 99.0f);
    route->setSource(source);
    route->setDestination(cistern);
    route->setPump(pump);
    route->update(100);
    assert(route->getRouteState() == Terra_RouteState_Requested || route->getRouteState() == Terra_RouteState_Active);

    auto temperature = controller.addRemoteSensor(Terra_SensorType_Temperature, Terra_UnitsType_Celsius, "Source Temp");
    auto store = controller.addThermalReservoir("Thermal Store");
    auto circulator = controller.addCirculatorRelay(6, false, "Circulator");
    auto loop = controller.addThermalLoop("Thermal Loop");
    assert(temperature && store && circulator && loop);
    temperature->receiveReport(70.0f, Terra_UnitsType_Celsius, 100, true);
    store->setTemperature(50.0f);
    loop->setSourceTemperatureSensor(temperature);
    loop->setThermalReservoir(store);
    loop->setCirculator(circulator);
    loop->configure(8.0f, 3.0f, 80.0f);
    loop->update(101);
    assert(loop->isRunning());

    std::cout << "PASS Terraduino automation" << std::endl;
    return 0;
}
