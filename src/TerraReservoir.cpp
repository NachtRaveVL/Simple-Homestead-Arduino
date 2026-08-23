/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Reservoirs
*/

#include "Terraduino.h"
#include "TerraUtils.h"
#include <string.h>

TerraReservoir *newReservoirObjectFromData(const TerraReservoirData *dataIn)
{
    TERRA_SOFT_ASSERT(dataIn && dataIn->isObjectData() && dataIn->id.object.idType == (tid_t)Terra_ObjectType_Reservoir,
                      F("Invalid reservoir data"));
    if (!dataIn || !dataIn->isObjectData() || dataIn->id.object.idType != (tid_t)Terra_ObjectType_Reservoir) { return nullptr; }

    switch (dataIn->id.object.classType) {
        case (tid_t)TerraReservoir::Base:
            return new TerraReservoir(dataIn);
        case (tid_t)TerraReservoir::WaterStorage:
            return new TerraWaterStorage((const TerraWaterStorageData *)dataIn);
        case (tid_t)TerraReservoir::Cistern:
            return new TerraCistern((const TerraCisternData *)dataIn);
        case (tid_t)TerraReservoir::WaterSource:
            return new TerraWaterSource((const TerraWaterSourceData *)dataIn);
        case (tid_t)TerraReservoir::ThermalStore:
            return new TerraThermalStore((const TerraThermalStoreData *)dataIn);
        default:
            return nullptr;
    }
}

TerraReservoir::TerraReservoir(Terra_ReservoirType type, tposi_t reservoirIndex, const TerraString &name, int classTypeIn)
    : TerraObject(TerraIdentity(type, reservoirIndex), name), classType(static_cast<decltype(Base)>(classTypeIn)),
      _state(Terra_ResourceState_Reserve), _level(0.0f), _reserveLevel(10.0f), _lowLevel(25.0f), _highLevel(90.0f)
{ ; }

TerraReservoir::TerraReservoir(const TerraReservoirData *dataIn)
    : TerraObject(dataIn), classType(static_cast<decltype(Base)>(dataIn ? (int)dataIn->id.object.classType : (int)Unknown)),
      _state(Terra_ResourceState_Reserve),
      _level(dataIn ? dataIn->level : 0.0f),
      _reserveLevel(dataIn ? dataIn->reserveLevel : 10.0f),
      _lowLevel(dataIn ? dataIn->lowLevel : 25.0f),
      _highLevel(dataIn ? dataIn->highLevel : 90.0f)
{
    updateState();
}

bool TerraReservoir::setThresholds(float reserveLevel, float lowLevel, float highLevel)
{
    if (reserveLevel < 0.0f || reserveLevel > lowLevel || lowLevel >= highLevel || highLevel > 100.0f) return false;
    _reserveLevel = reserveLevel;
    _lowLevel = lowLevel;
    _highLevel = highLevel;
    updateState();
    bumpRevisionIfNeeded();
    return true;
}

void TerraReservoir::setLevel(float level)
{
    float constrainedLevel = constrain(level, 0.0f, 100.0f);
    if (!isFPEqual(_level, constrainedLevel)) {
        _level = constrainedLevel;
        updateState();
        bumpRevisionIfNeeded();
    }
}

void TerraReservoir::setFault(const TerraString &message)
{
    TerraObject::setFault(message);
    updateState();
}

void TerraReservoir::clearFault()
{
    TerraObject::clearFault();
    updateState();
}

void TerraReservoir::updateState()
{
    if (_fault) { _state = Terra_ResourceState_Fault; }
    else if (_level <= _reserveLevel) { _state = Terra_ResourceState_Reserve; }
    else if (_level <= _lowLevel) { _state = Terra_ResourceState_Low; }
    else if (_level >= _highLevel) { _state = Terra_ResourceState_High; }
    else { _state = Terra_ResourceState_Normal; }
}

TerraData *TerraReservoir::allocateData() const
{
    return _allocateDataForObjType((int8_t)_id.type, (int8_t)classType);
}

void TerraReservoir::saveToData(TerraData *dataOut) const
{
    TerraObject::saveToData(dataOut);
    dataOut->id.object.classType = (tid_t)classType;
    auto data = static_cast<TerraReservoirData *>(dataOut);
    data->level = _level;
    data->reserveLevel = _reserveLevel;
    data->lowLevel = _lowLevel;
    data->highLevel = _highLevel;
}


TerraReservoirData::TerraReservoirData()
    : TerraObjectData(), level(0.0f), reserveLevel(10.0f), lowLevel(25.0f), highLevel(90.0f)
{
    _size = sizeof(*this);
    id.object.idType = (tid_t)Terra_ObjectType_Reservoir;
    id.object.objType = (tid_t)Terra_ReservoirType_Undefined;
    id.object.posIndex = TERRA_POS_SEARCH_FROMBEG;
    id.object.classType = (tid_t)TerraReservoir::Base;
}

void TerraReservoirData::toJSONObject(JsonObject &objectOut) const
{
    TerraObjectData::toJSONObject(objectOut);
    objectOut["level"] = level;
    objectOut["reserveLevel"] = reserveLevel;
    objectOut["lowLevel"] = lowLevel;
    objectOut["highLevel"] = highLevel;
}

void TerraReservoirData::fromJSONObject(JsonObjectConst &objectIn)
{
    TerraObjectData::fromJSONObject(objectIn);
    level = objectIn["level"] | level;
    reserveLevel = objectIn["reserveLevel"] | reserveLevel;
    lowLevel = objectIn["lowLevel"] | lowLevel;
    highLevel = objectIn["highLevel"] | highLevel;
}

TerraWaterStorageData::TerraWaterStorageData()
    : TerraReservoirData(), capacityLiters(0.0f), levelSensor{0}
{
    _size = sizeof(*this);
    id.object.objType = (tid_t)Terra_ReservoirType_Water;
    id.object.classType = (tid_t)TerraReservoir::WaterStorage;
}

void TerraWaterStorageData::toJSONObject(JsonObject &objectOut) const
{
    TerraReservoirData::toJSONObject(objectOut);
    objectOut["capacityLiters"] = capacityLiters;
    if (levelSensor[0]) { objectOut["levelSensor"] = levelSensor; }
}

void TerraWaterStorageData::fromJSONObject(JsonObjectConst &objectIn)
{
    TerraReservoirData::fromJSONObject(objectIn);
    capacityLiters = objectIn["capacityLiters"] | capacityLiters;
    const char *levelSensorIn = objectIn["levelSensor"] | nullptr;
    if (levelSensorIn) {
        strncpy(levelSensor, levelSensorIn, TERRA_NAME_MAXSIZE - 1);
        levelSensor[TERRA_NAME_MAXSIZE - 1] = '\0';
    }
}

TerraCisternData::TerraCisternData()
    : TerraWaterStorageData(), fillStartPercent(35.0f), fillStopPercent(90.0f), overflowPercent(99.0f)
{
    _size = sizeof(*this);
    id.object.classType = (tid_t)TerraReservoir::Cistern;
}

void TerraCisternData::toJSONObject(JsonObject &objectOut) const
{
    TerraWaterStorageData::toJSONObject(objectOut);
    objectOut["fillStartPercent"] = fillStartPercent;
    objectOut["fillStopPercent"] = fillStopPercent;
    objectOut["overflowPercent"] = overflowPercent;
}

void TerraCisternData::fromJSONObject(JsonObjectConst &objectIn)
{
    TerraWaterStorageData::fromJSONObject(objectIn);
    fillStartPercent = objectIn["fillStartPercent"] | fillStartPercent;
    fillStopPercent = objectIn["fillStopPercent"] | fillStopPercent;
    overflowPercent = objectIn["overflowPercent"] | overflowPercent;
}


TerraWaterSourceData::TerraWaterSourceData()
    : TerraReservoirData(), priority(0), available(true), maximumFlowLpm(0.0f), levelSensor{0}
{
    _size = sizeof(*this);
    id.object.objType = (tid_t)Terra_ReservoirType_Water;
    id.object.classType = (tid_t)TerraReservoir::WaterSource;
}

void TerraWaterSourceData::toJSONObject(JsonObject &objectOut) const
{
    TerraReservoirData::toJSONObject(objectOut);
    objectOut["priority"] = priority;
    objectOut["available"] = available;
    objectOut["maximumFlowLpm"] = maximumFlowLpm;
    if (levelSensor[0]) { objectOut["levelSensor"] = levelSensor; }
}

void TerraWaterSourceData::fromJSONObject(JsonObjectConst &objectIn)
{
    TerraReservoirData::fromJSONObject(objectIn);
    priority = objectIn["priority"] | priority;
    available = objectIn["available"] | available;
    maximumFlowLpm = objectIn["maximumFlowLpm"] | maximumFlowLpm;
    const char *levelSensorIn = objectIn["levelSensor"] | nullptr;
    if (levelSensorIn) {
        strncpy(levelSensor, levelSensorIn, TERRA_NAME_MAXSIZE - 1);
        levelSensor[TERRA_NAME_MAXSIZE - 1] = '\0';
    }
}

TerraThermalStoreData::TerraThermalStoreData()
    : TerraReservoirData(), temperatureC(0.0f), minimumTargetC(0.0f), maximumTargetC(80.0f),
      absoluteMaximumC(95.0f), temperatureSensor{0}
{
    _size = sizeof(*this);
    id.object.objType = (tid_t)Terra_ReservoirType_Thermal;
    id.object.classType = (tid_t)TerraReservoir::ThermalStore;
}

void TerraThermalStoreData::toJSONObject(JsonObject &objectOut) const
{
    TerraReservoirData::toJSONObject(objectOut);
    objectOut["temperatureC"] = temperatureC;
    objectOut["minimumTargetC"] = minimumTargetC;
    objectOut["maximumTargetC"] = maximumTargetC;
    objectOut["absoluteMaximumC"] = absoluteMaximumC;
    if (temperatureSensor[0]) { objectOut["temperatureSensor"] = temperatureSensor; }
}

void TerraThermalStoreData::fromJSONObject(JsonObjectConst &objectIn)
{
    TerraReservoirData::fromJSONObject(objectIn);
    temperatureC = objectIn["temperatureC"] | temperatureC;
    minimumTargetC = objectIn["minimumTargetC"] | minimumTargetC;
    maximumTargetC = objectIn["maximumTargetC"] | maximumTargetC;
    absoluteMaximumC = objectIn["absoluteMaximumC"] | absoluteMaximumC;
    const char *temperatureSensorIn = objectIn["temperatureSensor"] | nullptr;
    if (temperatureSensorIn) {
        strncpy(temperatureSensor, temperatureSensorIn, TERRA_NAME_MAXSIZE - 1);
        temperatureSensor[TERRA_NAME_MAXSIZE - 1] = '\0';
    }
}
