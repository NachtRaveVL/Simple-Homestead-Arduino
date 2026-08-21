/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Thermal Management
*/

#include "TerraThermal.h"
#include "TerraCoreLogic.h"

TerraThermalStore::TerraThermalStore(uint32_t key, const TerraString &name)
    : TerraResource(Terra_ResourceType_Thermal, key, name),
      _temperatureC(0.0f), _minimumTargetC(0.0f),
      _maximumTargetC(80.0f), _absoluteMaximumC(95.0f),
      _temperatureSensor(this)
{
    _objectType = Terra_ObjectType_ThermalStore;
}

bool TerraThermalStore::setTargetRange(float minimumC, float maximumC)
{
    if (minimumC > maximumC || maximumC > _absoluteMaximumC) { return false; }

    _minimumTargetC = minimumC;
    _maximumTargetC = maximumC;
    return true;
}

bool TerraThermalStore::setAbsoluteMaximum(float maximumC)
{
    if (maximumC < _maximumTargetC) { return false; }

    _absoluteMaximumC = maximumC;
    return true;
}

void TerraThermalStore::update(uint32_t now)
{
    if (!_temperatureSensor.isSet()) { return; }

    TerraMeasurement measurement = _temperatureSensor.getMeasurement(now, true);
    if (measurement.valid && measurement.unit == Terra_Unit_Celsius) {
        setTemperature(measurement.value);
    }
}

void TerraThermalStore::unresolveAny(TerraObject *object)
{
    _temperatureSensor.unresolveAny(object);
    TerraResource::unresolveAny(object);
}


TerraThermalLoop::TerraThermalLoop(uint32_t key, const TerraString &name)
    : TerraObject(Terra_ObjectType_ThermalLoop, key, name),
      _onDifferentialC(8.0f), _offDifferentialC(3.0f),
      _maxStoreTempC(80.0f), _running(false),
      _balancer(this)
{ ; }

bool TerraThermalLoop::configure(float onDifferentialC, float offDifferentialC, float maxStoreTempC)
{
    if (offDifferentialC < 0.0f || onDifferentialC <= offDifferentialC) { return false; }

    _onDifferentialC = onDifferentialC;
    _offDifferentialC = offDifferentialC;
    _maxStoreTempC = maxStoreTempC;
    return true;
}

void TerraThermalLoop::setEnabled(bool enabled)
{
    if (!enabled) { _running = false; }
    TerraObject::setEnabled(enabled);
}

bool TerraThermalLoop::shouldCirculate(float sourceTempC, float storeTempC) const
{
    return terraThermalLoopShouldRun(sourceTempC, storeTempC,
                                     _onDifferentialC, _offDifferentialC,
                                     _maxStoreTempC, _running);
}

void TerraThermalLoop::setRunning(bool running)
{
    _running = running && _enabled && !_fault;
}

void TerraThermalLoop::update(uint32_t now)
{
    _balancer.update(now);
}

void TerraThermalLoop::unresolveAny(TerraObject *object)
{
    _balancer.unresolveAny(object);
    TerraObject::unresolveAny(object);
}
