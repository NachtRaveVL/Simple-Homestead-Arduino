/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Thermal Management
*/

#include "Terraduino.h"
#include <string.h>

TerraThermalStore::TerraThermalStore(tposi_t storeIndex, const TerraString &name)
    : TerraReservoir(TerraIdentity(Terra_ObjectType_ThermalStore, storeIndex), name),
      _temperatureC(0.0f), _minimumTargetC(0.0f),
      _maximumTargetC(80.0f), _absoluteMaximumC(95.0f),
      _temperatureSensor(this)
{ ; }

TerraThermalStore::TerraThermalStore(const TerraThermalStoreData *dataIn)
    : TerraReservoir(dataIn),
      _temperatureC(dataIn ? dataIn->temperatureC : 0.0f),
      _minimumTargetC(dataIn ? dataIn->minimumTargetC : 0.0f),
      _maximumTargetC(dataIn ? dataIn->maximumTargetC : 80.0f),
      _absoluteMaximumC(dataIn ? dataIn->absoluteMaximumC : 95.0f),
      _temperatureSensor(this)
{
    if (dataIn && dataIn->temperatureSensor[0]) { _temperatureSensor.initObject(dataIn->temperatureSensor); }
}

bool TerraThermalStore::setTargetRange(float minimumC, float maximumC)
{
    if (minimumC > maximumC || maximumC > _absoluteMaximumC) { return false; }

    _minimumTargetC = minimumC;
    _maximumTargetC = maximumC;
    bumpRevisionIfNeeded();
    return true;
}

bool TerraThermalStore::setAbsoluteMaximum(float maximumC)
{
    if (maximumC < _maximumTargetC) { return false; }

    _absoluteMaximumC = maximumC;
    bumpRevisionIfNeeded();
    return true;
}

void TerraThermalStore::update(uint32_t now)
{
    TerraReservoir::update(now);
    if (!_temperatureSensor.isSet()) { return; }

    TerraSingleMeasurement measurement = _temperatureSensor.getMeasurement(now, true);
    if (measurement.isSet()) {
        if (measurement.units != Terra_Unit_Celsius) { measurement.toUnits(Terra_Unit_Celsius); }
        if (measurement.units == Terra_Unit_Celsius) { setTemperature(measurement.value); }
    }
}

void TerraThermalStore::unresolveAny(TerraObject *object)
{
    _temperatureSensor.unresolveAny(object);
    TerraReservoir::unresolveAny(object);
}

TerraData *TerraThermalStore::allocateData() const
{
    return new TerraThermalStoreData();
}

void TerraThermalStore::saveToData(TerraData *dataOut) const
{
    TerraReservoir::saveToData(dataOut);
    auto data = static_cast<TerraThermalStoreData *>(dataOut);
    data->temperatureC = _temperatureC;
    data->minimumTargetC = _minimumTargetC;
    data->maximumTargetC = _maximumTargetC;
    data->absoluteMaximumC = _absoluteMaximumC;
    if (_temperatureSensor.isSet()) {
        strncpy(data->temperatureSensor, _temperatureSensor.getKeyString().c_str(), TERRA_NAME_MAXSIZE - 1);
        data->temperatureSensor[TERRA_NAME_MAXSIZE - 1] = '\0';
    }
}


TerraThermalLoop::TerraThermalLoop(tposi_t loopIndex, const TerraString &name)
    : TerraObject(TerraIdentity(Terra_ObjectType_ThermalLoop, loopIndex), name),
      _onDifferentialC(8.0f), _offDifferentialC(3.0f),
      _maxStoreTempC(80.0f), _running(false),
      _balancer(this)
{ ; }

TerraThermalLoop::TerraThermalLoop(const TerraThermalLoopData *dataIn)
    : TerraObject(dataIn),
      _onDifferentialC(dataIn ? dataIn->onDifferentialC : 8.0f),
      _offDifferentialC(dataIn ? dataIn->offDifferentialC : 3.0f),
      _maxStoreTempC(dataIn ? dataIn->maxStoreTempC : 80.0f), _running(false), _balancer(this)
{
    if (dataIn) {
        if (dataIn->sourceTemperatureSensor[0]) { _balancer.getSourceTemperatureAttachment().initObject(dataIn->sourceTemperatureSensor); }
        if (dataIn->store[0]) { _balancer.getStoreAttachment().initObject(dataIn->store); }
        if (dataIn->circulator[0]) { _balancer.getCirculatorAttachment().initObject(dataIn->circulator); }
    }
}

bool TerraThermalLoop::configure(float onDifferentialC, float offDifferentialC, float maxStoreTempC)
{
    if (offDifferentialC < 0.0f || onDifferentialC <= offDifferentialC) { return false; }

    _onDifferentialC = onDifferentialC;
    _offDifferentialC = offDifferentialC;
    _maxStoreTempC = maxStoreTempC;
    bumpRevisionIfNeeded();
    return true;
}

void TerraThermalLoop::setEnabled(bool enabled)
{
    if (!enabled) { _running = false; }
    TerraObject::setEnabled(enabled);
}

bool TerraThermalLoop::shouldCirculate(float sourceTempC, float storeTempC) const
{
    if (storeTempC >= _maxStoreTempC) { return false; }

    float differential = sourceTempC - storeTempC;
    return _running ? differential > _offDifferentialC : differential >= _onDifferentialC;
}

void TerraThermalLoop::setRunning(bool running)
{
    _running = running && _enabled && !_fault;
}

void TerraThermalLoop::update(uint32_t now)
{
    TerraObject::update(now);
    _balancer.update(now);
}

void TerraThermalLoop::unresolveAny(TerraObject *object)
{
    _balancer.unresolveAny(object);
    TerraObject::unresolveAny(object);
}

TerraData *TerraThermalLoop::allocateData() const
{
    return new TerraThermalLoopData();
}

void TerraThermalLoop::saveToData(TerraData *dataOut) const
{
    TerraObject::saveToData(dataOut);
    auto data = static_cast<TerraThermalLoopData *>(dataOut);
    auto copyAttachment = [](char *destination, const TerraAttachment &attachment) {
        if (attachment.isSet()) {
            strncpy(destination, attachment.getKeyString().c_str(), TERRA_NAME_MAXSIZE - 1);
            destination[TERRA_NAME_MAXSIZE - 1] = '\0';
        }
    };
    copyAttachment(data->sourceTemperatureSensor, _balancer.getSourceTemperatureAttachment());
    copyAttachment(data->store, _balancer.getStoreAttachment());
    copyAttachment(data->circulator, _balancer.getCirculatorAttachment());
    data->onDifferentialC = _onDifferentialC;
    data->offDifferentialC = _offDifferentialC;
    data->maxStoreTempC = _maxStoreTempC;
}
