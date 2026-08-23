/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Thermal Management
*/

#include "Terraduino.h"
#include <string.h>

TerraThermalStore::TerraThermalStore(tposi_t storeIndex, const TerraString &name)
    : TerraReservoir(Terra_ReservoirType_Thermal, storeIndex, name, TerraReservoir::ThermalStore),
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
        if (measurement.units != Terra_UnitsType_Temperature_Celsius) { measurement.toUnits(Terra_UnitsType_Temperature_Celsius); }
        if (measurement.units == Terra_UnitsType_Temperature_Celsius) { setTemperature(measurement.value); }
    }
}

void TerraThermalStore::unresolveAny(TerraObject *object)
{
    _temperatureSensor.unresolveAny(object);
    TerraReservoir::unresolveAny(object);
}

TerraData *TerraThermalStore::allocateData() const
{
    return _allocateDataForObjType((int8_t)_id.type, (int8_t)classType);
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
    : TerraObject(TerraIdentity(Terra_ObjectType_ThermalLoop, loopIndex), name), classType(Loop),
      _onDifferentialC(8.0f), _offDifferentialC(3.0f),
      _maxStoreTempC(80.0f), _running(false),
      _balancer(this)
{ ; }

TerraThermalLoop::TerraThermalLoop(const TerraThermalLoopData *dataIn)
    : TerraObject(dataIn), classType(static_cast<decltype(Loop)>(dataIn ? (int)dataIn->id.object.classType : (int)Unknown)),
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
    return _allocateDataForObjType((int8_t)_id.type, (int8_t)classType);
}

void TerraThermalLoop::saveToData(TerraData *dataOut) const
{
    TerraObject::saveToData(dataOut);
    dataOut->id.object.classType = (tid_t)classType;
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


TerraThermalLoopData::TerraThermalLoopData()
    : TerraObjectData(), sourceTemperatureSensor{0}, store{0}, circulator{0},
      onDifferentialC(8.0f), offDifferentialC(3.0f), maxStoreTempC(80.0f)
{
    _size = sizeof(*this);
    id.object.idType = (tid_t)Terra_ObjectType_ThermalLoop;
    id.object.objType = 0;
    id.object.posIndex = TERRA_POS_SEARCH_FROMBEG;
    id.object.classType = (tid_t)TerraThermalLoop::Loop;
}

void TerraThermalLoopData::toJSONObject(JsonObject &objectOut) const
{
    TerraObjectData::toJSONObject(objectOut);
    if (sourceTemperatureSensor[0]) { objectOut["sourceTemperatureSensor"] = sourceTemperatureSensor; }
    if (store[0]) { objectOut["store"] = store; }
    if (circulator[0]) { objectOut["circulator"] = circulator; }
    objectOut["onDifferentialC"] = onDifferentialC;
    objectOut["offDifferentialC"] = offDifferentialC;
    objectOut["maxStoreTempC"] = maxStoreTempC;
}

void TerraThermalLoopData::fromJSONObject(JsonObjectConst &objectIn)
{
    TerraObjectData::fromJSONObject(objectIn);
    auto copyString = [](char *destinationOut, JsonVariantConst sourceIn) {
        const char *value = sourceIn | nullptr;
        if (value) {
            strncpy(destinationOut, value, TERRA_NAME_MAXSIZE - 1);
            destinationOut[TERRA_NAME_MAXSIZE - 1] = '\0';
        }
    };
    copyString(sourceTemperatureSensor, objectIn["sourceTemperatureSensor"]);
    copyString(store, objectIn["store"]);
    copyString(circulator, objectIn["circulator"]);
    onDifferentialC = objectIn["onDifferentialC"] | onDifferentialC;
    offDifferentialC = objectIn["offDifferentialC"] | offDifferentialC;
    maxStoreTempC = objectIn["maxStoreTempC"] | maxStoreTempC;
}
