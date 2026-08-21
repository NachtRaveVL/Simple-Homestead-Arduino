/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Balancers
*/

#include "TerraBalancers.h"
#include "TerraActuators.h"
#include "TerraSensors.h"
#include "TerraThermal.h"
#include "TerraWater.h"

TerraWaterBalancer::TerraWaterBalancer(TerraWaterRoute *route)
    : _route(route), _source(route), _destination(route),
      _pump(route), _flowSensor(route), _lastDecision()
{ ; }

void TerraWaterBalancer::setParent(TerraWaterRoute *route)
{
    _route = route;
    _source.setParent(route);
    _destination.setParent(route);
    _pump.setParent(route);
    _flowSensor.setParent(route);
}

void TerraWaterBalancer::update(uint32_t now)
{
    if (!_route || !_route->isEnabled() || _route->hasFault()) {
        _pump.disableActivation();
        return;
    }

    SharedPtr<TerraWaterSource> source = _source.getObject();
    SharedPtr<TerraWaterStorage> destination = _destination.getObject();
    if (!source || !destination || !_pump.isSet()) {
        _pump.disableActivation();
        return;
    }

    _lastDecision = _route->decide(*source, *destination);
    bool run = _lastDecision.run;
    bool pumpWasActive = _pump.isActivated();

    if (_flowSensor.isSet()) {
        TerraMeasurement flow = _flowSensor.getMeasurement(now, true);
        if (!flow.valid || flow.unit != Terra_Unit_LitersPerMinute) {
            run = false;
        } else if ((run && pumpWasActive && !_route->validateFlow(flow.value, true)) ||
                   (!run && !_route->validateFlow(flow.value, false))) {
            run = false;
        }
    }

    if (run) { _pump.setupActivation(); _pump.enableActivation(); }
    else { _pump.disableActivation(); }
}

void TerraWaterBalancer::unresolveAny(TerraObject *object)
{
    _source.unresolveAny(object);
    _destination.unresolveAny(object);
    _pump.unresolveAny(object);
    _flowSensor.unresolveAny(object);
}

TerraThermalBalancer::TerraThermalBalancer(TerraThermalLoop *loop)
    : _loop(loop), _sourceTemperature(loop),
      _store(loop), _circulator(loop)
{ ; }

void TerraThermalBalancer::setParent(TerraThermalLoop *loop)
{
    _loop = loop;
    _sourceTemperature.setParent(loop);
    _store.setParent(loop);
    _circulator.setParent(loop);
}

void TerraThermalBalancer::update(uint32_t now)
{
    if (!_loop || !_loop->isEnabled() || _loop->hasFault()) {
        _circulator.disableActivation();
        if (_loop) { _loop->setRunning(false); }
        return;
    }

    SharedPtr<TerraThermalStore> store = _store.getObject();
    if (!store || !_sourceTemperature.isSet() || !_circulator.isSet()) {
        _circulator.disableActivation();
        _loop->setRunning(false);
        return;
    }

    TerraMeasurement source = _sourceTemperature.getMeasurement(now, true);
    if (!source.valid || source.unit != Terra_Unit_Celsius) {
        _circulator.disableActivation();
        _loop->setRunning(false);
        return;
    }

    bool run = _loop->shouldCirculate(source.value, store->getTemperature());
    _loop->setRunning(run);
    if (run) { _circulator.setupActivation(); _circulator.enableActivation(); }
    else { _circulator.disableActivation(); }
}

void TerraThermalBalancer::unresolveAny(TerraObject *object)
{
    _sourceTemperature.unresolveAny(object);
    _store.unresolveAny(object);
    _circulator.unresolveAny(object);
}
